#include "EmberPlayerController.h"
#include "ChaosModularVehicle/ModularVehicleBaseComponent.h"
#include "EmberCharacter.h"
#include "EmberLog.h"
#include "EmberVehicleSeatComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Pawn.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"

void AEmberPlayerController::BeginPlay()
{
    Super::BeginPlay();
    ArmGameplayInput();
}

void AEmberPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    if (UModularVehicleBaseComponent* VehicleSimulation = GetVehicleSimulation())
    {
        VehicleSimulation->SetLocallyControlled(true);
        VehicleSimulation->SetupInputConfiguration();
    }
    UpdateVehicleBridgeInputConsumption();
    ArmGameplayInput();
}

void AEmberPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (!InputComponent) return;
    // These controller-level bindings remain active while a Blueprint vehicle
    // is possessed. On foot, the callbacks intentionally no-op and the
    // character's existing bindings remain authoritative.
    // Controller input components are evaluated before the possessed pawn. A
    // consuming controller binding therefore prevents the on-foot character
    // from ever receiving the same mapping, even when the vehicle callback
    // correctly no-ops. Keep these vehicle bridge bindings non-consuming so
    // WASD, mouse look, jump and interact continue down to AEmberCharacter.
    FInputAxisBinding& MoveForwardBinding = InputComponent->BindAxis(
        TEXT("MoveForward"), this, &AEmberPlayerController::ApplyVehicleLongitudinal);
    MoveForwardBinding.bConsumeInput = false;
    FInputAxisBinding& MoveRightBinding = InputComponent->BindAxis(
        TEXT("MoveRight"), this, &AEmberPlayerController::ApplyVehicleSteering);
    MoveRightBinding.bConsumeInput = false;
    FInputAxisBinding& TurnBinding = InputComponent->BindAxis(
        TEXT("Turn"), this, &AEmberPlayerController::ApplyVehicleLookYaw);
    TurnBinding.bConsumeInput = false;
    FInputAxisBinding& LookUpBinding = InputComponent->BindAxis(
        TEXT("LookUp"), this, &AEmberPlayerController::ApplyVehicleLookPitch);
    LookUpBinding.bConsumeInput = false;

    FInputActionBinding& HandbrakePressedBinding = InputComponent->BindAction(
        TEXT("Jump"), IE_Pressed, this, &AEmberPlayerController::VehicleHandbrakePressed);
    HandbrakePressedBinding.bConsumeInput = false;
    FInputActionBinding& HandbrakeReleasedBinding = InputComponent->BindAction(
        TEXT("Jump"), IE_Released, this, &AEmberPlayerController::VehicleHandbrakeReleased);
    HandbrakeReleasedBinding.bConsumeInput = false;
    FInputActionBinding& VehicleInteractBinding = InputComponent->BindAction(
        TEXT("Interact"), IE_Pressed, this, &AEmberPlayerController::VehicleInteractPressed);
    VehicleInteractBinding.bConsumeInput = false;
    UpdateVehicleBridgeInputConsumption();
}

void AEmberPlayerController::UpdateVehicleBridgeInputConsumption()
{
    if (!InputComponent) return;
    const bool bConsumeForVehicle = IsDrivingVehicle();
    for (FInputAxisBinding& Binding : InputComponent->AxisBindings)
    {
        if (Binding.AxisName == TEXT("MoveForward") || Binding.AxisName == TEXT("MoveRight")
            || Binding.AxisName == TEXT("Turn") || Binding.AxisName == TEXT("LookUp"))
        {
            Binding.bConsumeInput = bConsumeForVehicle;
        }
    }
    for (int32 Index = 0; Index < InputComponent->GetNumActionBindings(); ++Index)
    {
        FInputActionBinding& Binding = InputComponent->GetActionBinding(Index);
        if (Binding.GetActionName() == TEXT("Jump") || Binding.GetActionName() == TEXT("Interact"))
        {
            Binding.bConsumeInput = bConsumeForVehicle;
        }
    }
    UE_LOG(LogEmberCombat, Log, TEXT("Vehicle input bridge: driving=%s consume=%s"),
        bConsumeForVehicle ? TEXT("true") : TEXT("false"),
        bConsumeForVehicle ? TEXT("true") : TEXT("false"));
}

void AEmberPlayerController::ResetVehicleForPlayerControl(APawn* VehiclePawn)
{
    if (!VehiclePawn) return;
    // The example car owns Enhanced Input Blueprint callbacks that continue to
    // publish zero-valued controls every frame. They race the controller bridge
    // and erase A/D steering immediately after it is submitted. Remove only
    // the possessed pawn's input component from this controller's stack;
    // AEmberPlayerController remains active and is the sole driving authority.
    VehiclePawn->DisableInput(this);
    if (UPrimitiveComponent* PhysicsRoot = Cast<UPrimitiveComponent>(VehiclePawn->GetRootComponent()))
    {
        PhysicsRoot->SetPhysicsLinearVelocity(FVector::ZeroVector);
        PhysicsRoot->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }
    if (UModularVehicleBaseComponent* Simulation =
        VehiclePawn->FindComponentByClass<UModularVehicleBaseComponent>())
    {
        Simulation->SetLocallyControlled(true);
        Simulation->SetupInputConfiguration();
        Simulation->SetInputAxis1D(TEXT("Throttle"), 0.0);
        Simulation->SetInputAxis1D(TEXT("Steering"), 0.0);
        Simulation->SetInputAxis1D(TEXT("Brake"), 1.0);
        Simulation->SetInputBool(TEXT("Reverse"), false);
        Simulation->SetInputBool(TEXT("Handbrake"), true);
    }
    UE_LOG(LogEmberCombat, Log,
        TEXT("Vehicle control state reset: blueprintInput=disabled velocity=0 steering=0 handbrake=true"));
}

UEmberVehicleSeatComponent* AEmberPlayerController::GetVehicleSeatComponent() const
{
    return GetPawn() ? GetPawn()->FindComponentByClass<UEmberVehicleSeatComponent>() : nullptr;
}

bool AEmberPlayerController::IsDrivingVehicle() const
{
    const UEmberVehicleSeatComponent* Seat = GetVehicleSeatComponent();
    return Seat && Seat->GetDriverCharacter();
}

UModularVehicleBaseComponent* AEmberPlayerController::GetVehicleSimulation() const
{
    return GetPawn() ? GetPawn()->FindComponentByClass<UModularVehicleBaseComponent>() : nullptr;
}

bool AEmberPlayerController::EnterVehicle(APawn* VehiclePawn, AEmberCharacter* DriverCharacter)
{
    AController* ExistingVehicleController = VehiclePawn ? VehiclePawn->GetController() : nullptr;
    if (!VehiclePawn || !DriverCharacter || GetPawn() != DriverCharacter
        || !VehiclePawn->ActorHasTag(TEXT("EmberDriveableVehicle"))
        || (ExistingVehicleController && ExistingVehicleController->IsPlayerController()))
    {
        return false;
    }
    if (FVector::DistSquared(VehiclePawn->GetActorLocation(), DriverCharacter->GetActorLocation())
        > FMath::Square(650.0f))
    {
        return false;
    }

    UEmberVehicleSeatComponent* Seat = VehiclePawn->FindComponentByClass<UEmberVehicleSeatComponent>();
    if (!Seat)
    {
        Seat = NewObject<UEmberVehicleSeatComponent>(VehiclePawn, TEXT("EmberVehicleSeat"));
        Seat->RegisterComponent();
    }
    if (!Seat->SeatDriver(DriverCharacter)) return false;

    // Epic's placed example car may create an initialization controller. It is
    // not an occupant; release it immediately before the local player takes
    // authority over the real Chaos simulation.
    if (ExistingVehicleController) ExistingVehicleController->UnPossess();

    LastVehicleEnterTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
    Possess(VehiclePawn);
    ResetVehicleForPlayerControl(VehiclePawn);
    // The example Blueprint's possession event may reactivate its authored
    // overhead camera. Resolve camera ownership after possession so cooked and
    // Editor builds deterministically select Ember's third-person chase view.
    Seat->ActivateVehicleCamera();
    SetViewTarget(VehiclePawn);
    bVehicleHandbrake = true;
    bVehicleDriveInputConfirmed = false;
    bVehicleSteeringInputConfirmed = false;
    Seat->SetControlState(0.0f, 0.0f, 1.0f, true);
    return GetPawn() == VehiclePawn;
}

void AEmberPlayerController::ExitVehicle()
{
    UEmberVehicleSeatComponent* Seat = GetVehicleSeatComponent();
    UModularVehicleBaseComponent* Simulation = GetVehicleSimulation();
    if (!Seat) return;
    if (Simulation)
    {
        Simulation->SetInputAxis1D(TEXT("Throttle"), 0.0);
        Simulation->SetInputAxis1D(TEXT("Steering"), 0.0);
        Simulation->SetInputAxis1D(TEXT("Brake"), 1.0);
        Simulation->SetInputBool(TEXT("Reverse"), false);
        Simulation->SetInputBool(TEXT("Handbrake"), true);
    }
    FVector ExitLocation;
    FRotator ExitRotation;
    AEmberCharacter* Driver = Seat->ReleaseDriver(ExitLocation, ExitRotation);
    if (!Driver) return;
    Driver->RestoreFromVehicle(ExitLocation, ExitRotation);
    Possess(Driver);
    Seat->DestroyComponent();
    bVehicleHandbrake = false;
    bVehicleDriveInputConfirmed = false;
    bVehicleSteeringInputConfirmed = false;
}

void AEmberPlayerController::ApplyVehicleLongitudinal(const float Value)
{
    UModularVehicleBaseComponent* Simulation = GetVehicleSimulation();
    UEmberVehicleSeatComponent* Seat = GetVehicleSeatComponent();
    if (!Simulation || !Seat) return;
    const float ForwardSpeed = Seat->GetForwardSpeedKPH();
    float Throttle = 0.0f;
    float Brake = 0.0f;
    bool bReverse = false;
    if (Value > KINDA_SMALL_NUMBER)
    {
        bVehicleHandbrake = false;
        if (ForwardSpeed < -2.0f) Brake = Value;
        else Throttle = Value;
    }
    else if (Value < -KINDA_SMALL_NUMBER)
    {
        bVehicleHandbrake = false;
        if (ForwardSpeed > 3.0f) Brake = -Value;
        else
        {
            Throttle = -Value;
            bReverse = true;
        }
    }
    Simulation->SetInputAxis1D(TEXT("Throttle"), Throttle);
    Simulation->SetInputAxis1D(TEXT("Brake"), Brake);
    Simulation->SetInputBool(TEXT("Reverse"), bReverse);
    Simulation->SetInputBool(TEXT("Handbrake"), bVehicleHandbrake);
    Seat->SetControlState(bReverse ? -Throttle : Throttle, Seat->GetSteeringInput(), Brake, bVehicleHandbrake);
    if (!bVehicleDriveInputConfirmed && !FMath::IsNearlyZero(Value))
    {
        bVehicleDriveInputConfirmed = true;
        UE_LOG(LogEmberCombat, Log,
            TEXT("Vehicle driving input confirmed: throttle=%.2f brake=%.2f reverse=%s"),
            Throttle, Brake, bReverse ? TEXT("true") : TEXT("false"));
    }
}

void AEmberPlayerController::ApplyVehicleSteering(const float Value)
{
    UModularVehicleBaseComponent* Simulation = GetVehicleSimulation();
    UEmberVehicleSeatComponent* Seat = GetVehicleSeatComponent();
    if (!Simulation || !Seat) return;
    const float Steering = FMath::Clamp(Value, -1.0f, 1.0f);
    Simulation->SetInputAxis1D(TEXT("Steering"), Steering);
    Seat->SetControlState(Seat->GetThrottleInput(), Steering, Seat->GetBrakeInput(), bVehicleHandbrake);
    if (!bVehicleSteeringInputConfirmed && FMath::Abs(Value) > 0.1f)
    {
        bVehicleSteeringInputConfirmed = true;
        UE_LOG(LogEmberCombat, Log, TEXT("Vehicle steering input confirmed: %.2f"), Value);
    }
}

void AEmberPlayerController::ApplyVehicleLookYaw(const float Value)
{
    if (UEmberVehicleSeatComponent* Seat = GetVehicleSeatComponent())
    {
        if (!FMath::IsNearlyZero(Value) && !IsLookInputIgnored())
        {
            Seat->AddCameraInput(Value, 0.0f);
        }
    }
}

void AEmberPlayerController::ApplyVehicleLookPitch(const float Value)
{
    if (UEmberVehicleSeatComponent* Seat = GetVehicleSeatComponent())
    {
        if (!FMath::IsNearlyZero(Value) && !IsLookInputIgnored())
        {
            Seat->AddCameraInput(0.0f, Value);
        }
    }
}

void AEmberPlayerController::VehicleHandbrakePressed()
{
    UModularVehicleBaseComponent* Simulation = GetVehicleSimulation();
    UEmberVehicleSeatComponent* Seat = GetVehicleSeatComponent();
    if (!Simulation || !Seat) return;
    bVehicleHandbrake = true;
    Simulation->SetInputBool(TEXT("Handbrake"), true);
    Seat->SetControlState(Seat->GetThrottleInput(), Seat->GetSteeringInput(), Seat->GetBrakeInput(), true);
}

void AEmberPlayerController::VehicleHandbrakeReleased()
{
    UModularVehicleBaseComponent* Simulation = GetVehicleSimulation();
    UEmberVehicleSeatComponent* Seat = GetVehicleSeatComponent();
    if (!Simulation || !Seat) return;
    bVehicleHandbrake = false;
    Simulation->SetInputBool(TEXT("Handbrake"), false);
    Seat->SetControlState(Seat->GetThrottleInput(), Seat->GetSteeringInput(), Seat->GetBrakeInput(), false);
}

void AEmberPlayerController::VehicleInteractPressed()
{
    if (!IsDrivingVehicle() || !GetWorld()) return;
    // The same E/F press that enters a car is visible to the controller input
    // stack after possession. Ignore that press so releasing/pressing again is
    // required to exit instead of immediately ejecting the player.
    if (GetWorld()->GetTimeSeconds() - LastVehicleEnterTime < 0.45) return;
    ExitVehicle();
}

void AEmberPlayerController::EmberVehicleSmoke()
{
#if !UE_BUILD_SHIPPING
    AEmberCharacter* Driver = Cast<AEmberCharacter>(GetPawn());
    if (!Driver || !GetWorld())
    {
        UE_LOG(LogEmberCombat, Error, TEXT("Vehicle smoke failed: no Ember driver"));
        return;
    }
    APawn* NearestVehicle = nullptr;
    float NearestDistanceSquared = TNumericLimits<float>::Max();
    for (TActorIterator<APawn> It(GetWorld()); It; ++It)
    {
        APawn* Candidate = *It;
        if (!Candidate || !Candidate->ActorHasTag(TEXT("EmberDriveableVehicle"))
            || (Candidate->GetController() && Candidate->GetController()->IsPlayerController())) continue;
        const float DistanceSquared = FVector::DistSquared(Driver->GetActorLocation(), Candidate->GetActorLocation());
        if (DistanceSquared < NearestDistanceSquared)
        {
            NearestVehicle = Candidate;
            NearestDistanceSquared = DistanceSquared;
        }
    }
    if (!NearestVehicle || !EnterVehicle(NearestVehicle, Driver))
    {
        UE_LOG(LogEmberCombat, Error, TEXT("Vehicle smoke failed: entry rejected"));
        return;
    }
    VehicleSmokeStartLocation = NearestVehicle->GetActorLocation();
    VehicleSmokeStartYaw = NearestVehicle->GetActorRotation().Yaw;
    VehicleSmokeRightTurnYaw = VehicleSmokeStartYaw;
    VehicleSmokeLeftTurnYaw = VehicleSmokeStartYaw;
    GetWorldTimerManager().SetTimer(VehicleSmokeIdleTimer, this,
        &AEmberPlayerController::ValidateVehicleSmokeIdle, 0.55f, false);
    GetWorldTimerManager().SetTimer(VehicleSmokeStartTimer, this,
        &AEmberPlayerController::BeginVehicleSmokeMotion, 0.75f, false);
    GetWorldTimerManager().SetTimer(VehicleSmokeReverseSteeringTimer, this,
        &AEmberPlayerController::ReverseVehicleSmokeSteering, 5.0f, false);
    GetWorldTimerManager().SetTimer(VehicleSmokeNeutralSteeringTimer, this,
        &AEmberPlayerController::NeutralizeVehicleSmokeSteering, 9.0f, false);
    GetWorldTimerManager().SetTimer(VehicleSmokeCaptureTimer, this,
        &AEmberPlayerController::CaptureVehicleSmoke, 10.0f, false);
    GetWorldTimerManager().SetTimer(VehicleSmokeExitTimer, this,
        &AEmberPlayerController::CompleteVehicleSmoke, 11.5f, false);
    UE_LOG(LogEmberCombat, Log, TEXT("Vehicle smoke started"));
#endif
}

void AEmberPlayerController::ValidateVehicleSmokeIdle()
{
    const APawn* Vehicle = GetPawn();
    if (!Vehicle || !IsDrivingVehicle()) return;
    const float IdleYawDelta = FMath::FindDeltaAngleDegrees(
        VehicleSmokeStartYaw, Vehicle->GetActorRotation().Yaw);
    if (FMath::Abs(IdleYawDelta) <= 1.0f)
    {
        UE_LOG(LogEmberCombat, Log,
            TEXT("Vehicle idle stability: yawDelta=%.2f degrees"), IdleYawDelta);
    }
    else
    {
        UE_LOG(LogEmberCombat, Error,
            TEXT("Vehicle idle stability failed: yawDelta=%.2f degrees"), IdleYawDelta);
    }
}

void AEmberPlayerController::BeginVehicleSmokeMotion()
{
    ApplyVehicleLongitudinal(1.0f);
    ApplyVehicleSteering(0.45f);
    UE_LOG(LogEmberCombat, Log, TEXT("Vehicle smoke steering phase: RIGHT"));
}

void AEmberPlayerController::ReverseVehicleSmokeSteering()
{
    const APawn* Vehicle = GetPawn();
    VehicleSmokeRightTurnYaw = Vehicle ? Vehicle->GetActorRotation().Yaw : VehicleSmokeStartYaw;
    ApplyVehicleSteering(-0.45f);
    UE_LOG(LogEmberCombat, Log, TEXT("Vehicle smoke steering phase: LEFT"));
}

void AEmberPlayerController::NeutralizeVehicleSmokeSteering()
{
    const APawn* Vehicle = GetPawn();
    VehicleSmokeLeftTurnYaw = Vehicle ? Vehicle->GetActorRotation().Yaw : VehicleSmokeRightTurnYaw;
    ApplyVehicleSteering(0.0f);
    UE_LOG(LogEmberCombat, Log, TEXT("Vehicle smoke steering phase: CENTER"));
}

void AEmberPlayerController::CaptureVehicleSmoke()
{
    ApplyVehicleLongitudinal(0.0f);
    ApplyVehicleSteering(0.0f);
    const APawn* Vehicle = GetPawn();
    const float Displacement = Vehicle
        ? FVector::Dist2D(VehicleSmokeStartLocation, Vehicle->GetActorLocation()) : 0.0f;
    const FVector VehicleLocation = Vehicle ? Vehicle->GetActorLocation() : FVector::ZeroVector;
    const float RightYawDelta = FMath::FindDeltaAngleDegrees(VehicleSmokeStartYaw, VehicleSmokeRightTurnYaw);
    const float LeftYawDelta = FMath::FindDeltaAngleDegrees(VehicleSmokeRightTurnYaw, VehicleSmokeLeftTurnYaw);
    UE_LOG(LogEmberCombat, Log,
        TEXT("Vehicle smoke capture: displacement=%.1f cm speed=%.1f KPH rightYaw=%.2f leftYaw=%.2f start=(%.1f,%.1f,%.1f) current=(%.1f,%.1f,%.1f)"),
        Displacement, GetVehicleSeatComponent() ? GetVehicleSeatComponent()->GetSpeedKPH() : 0.0f,
        RightYawDelta, LeftYawDelta,
        VehicleSmokeStartLocation.X, VehicleSmokeStartLocation.Y, VehicleSmokeStartLocation.Z,
        VehicleLocation.X, VehicleLocation.Y, VehicleLocation.Z);
    const FString ScreenshotPath = FPaths::ProjectSavedDir()
        / TEXT("Screenshots/VehicleDriveSmoke.png");
    FScreenshotRequest::RequestScreenshot(ScreenshotPath, true, false, false,
        FIntRect(), true);
    UE_LOG(LogEmberCombat, Log, TEXT("Vehicle smoke screenshot requested: %s"), *ScreenshotPath);
}

void AEmberPlayerController::CompleteVehicleSmoke()
{
    ExitVehicle();
    UE_LOG(LogEmberCombat, Log, TEXT("Vehicle smoke completed"));
}

void AEmberPlayerController::ArmGameplayInput()
{
    SetPause(false);
    bShowMouseCursor = false;
    // Ignore-input APIs are stack based. Never add a launch-time look lock:
    // BeginPlay and possession can overlap, and a missed timer would leave the
    // owner with a visible but completely static character. Normalizing the
    // stacks is safe for both on-foot and vehicle possession.
    ResetIgnoreLookInput();
    ResetIgnoreMoveInput();
    FInputModeGameOnly GameOnlyInput;
    GameOnlyInput.SetConsumeCaptureMouseDown(false);
    SetInputMode(GameOnlyInput);
    FocusGameplayViewport();
    GetWorldTimerManager().ClearTimer(GameplayInputActivationTimer);
    GetWorldTimerManager().SetTimer(
        GameplayInputActivationTimer,
        this,
        &AEmberPlayerController::ActivateGameplayInput,
        0.15f,
        false);
}

void AEmberPlayerController::ActivateGameplayInput()
{
    if (const APawn* ControlledPawn = GetPawn())
    {
        SetControlRotation(FRotator(0.0f, ControlledPawn->GetActorRotation().Yaw, 0.0f));
    }
    SetPause(false);
    bShowMouseCursor = false;
    ResetIgnoreLookInput();
    ResetIgnoreMoveInput();
    FInputModeGameOnly GameOnlyInput;
    GameOnlyInput.SetConsumeCaptureMouseDown(false);
    SetInputMode(GameOnlyInput);
    FocusGameplayViewport();
    UE_LOG(LogEmberCombat, Log,
        TEXT("Gameplay input activated; lookIgnored=%s moveIgnored=%s viewportFocused=%s"),
        IsLookInputIgnored() ? TEXT("true") : TEXT("false"),
        IsMoveInputIgnored() ? TEXT("true") : TEXT("false"),
        FSlateApplication::IsInitialized() && FSlateApplication::Get().GetUserFocusedWidget(0).IsValid()
            ? TEXT("true") : TEXT("false"));
}

void AEmberPlayerController::FocusGameplayViewport()
{
    if (!GEngine || !GEngine->GameViewport || !FSlateApplication::IsInitialized()) return;

    UGameViewportClient* ViewportClient = GEngine->GameViewport;
    ViewportClient->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
    ViewportClient->SetMouseLockMode(EMouseLockMode::LockOnCapture);
    if (const TSharedPtr<SViewport> ViewportWidget = ViewportClient->GetGameViewportWidget();
        ViewportWidget.IsValid())
    {
        FSlateApplication::Get().RegisterGameViewport(ViewportWidget.ToSharedRef());
        FSlateApplication::Get().SetAllUserFocusToGameViewport(EFocusCause::SetDirectly);
    }
}
