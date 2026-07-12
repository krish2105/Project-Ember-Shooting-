#include "EmberCharacter.h"
#include "Animation/AnimationAsset.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "EmberArmorComponent.h"
#include "EmberDamageReceiverComponent.h"
#include "EmberHealthComponent.h"
#include "EmberInteractionComponent.h"
#include "EmberInventoryComponent.h"
#include "EmberWeaponComponent.h"
#include "EmberWeaponDefinition.h"
#include "Engine/AssetManager.h"
#include "Engine/StaticMesh.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Sound/SoundWaveProcedural.h"
#include "UObject/ConstructorHelpers.h"

AEmberCharacter::AEmberCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    AutoPossessPlayer = EAutoReceiveInput::Player0;
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = ExplorationArmLength;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->SocketOffset = FVector(0.0f, ShoulderOffset, 70.0f);

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    Health = CreateDefaultSubobject<UEmberHealthComponent>(TEXT("Health"));
    Armor = CreateDefaultSubobject<UEmberArmorComponent>(TEXT("Armor"));
    DamageReceiver = CreateDefaultSubobject<UEmberDamageReceiverComponent>(TEXT("DamageReceiver"));
    Weapon = CreateDefaultSubobject<UEmberWeaponComponent>(TEXT("Weapon"));
    Inventory = CreateDefaultSubobject<UEmberInventoryComponent>(TEXT("Inventory"));
    Interaction = CreateDefaultSubobject<UEmberInteractionComponent>(TEXT("Interaction"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    WeaponBodyVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponBodyVisual"));
    WeaponBodyVisual->SetupAttachment(GetMesh(), TEXT("hand_r"));
    WeaponBodyVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponBodyVisual->SetRelativeLocation(FVector(12.0f, 2.0f, -1.0f));
    WeaponBodyVisual->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    WeaponBodyVisual->SetRelativeScale3D(FVector(0.55f, 0.08f, 0.10f));
    WeaponBarrelVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponBarrelVisual"));
    WeaponBarrelVisual->SetupAttachment(WeaponBodyVisual);
    WeaponBarrelVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponBarrelVisual->SetRelativeLocation(FVector(58.0f, 0.0f, 2.0f));
    WeaponBarrelVisual->SetRelativeScale3D(FVector(0.65f, 0.40f, 0.40f));
    if (CubeMesh.Succeeded())
    {
        WeaponBodyVisual->SetStaticMesh(CubeMesh.Object);
        WeaponBarrelVisual->SetStaticMesh(CubeMesh.Object);
    }
    MuzzleFlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("MuzzleFlashLight"));
    MuzzleFlashLight->SetupAttachment(WeaponBarrelVisual);
    MuzzleFlashLight->SetRelativeLocation(FVector(110.0f, 0.0f, 0.0f));
    MuzzleFlashLight->SetAttenuationRadius(450.0f);
    MuzzleFlashLight->SetLightColor(FLinearColor(1.0f, 0.35f, 0.04f));
    MuzzleFlashLight->SetIntensity(0.0f);
}

void AEmberCharacter::BeginPlay()
{
    Super::BeginPlay();
    IdleAnimation = LoadObject<UAnimationAsset>(nullptr,
        TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle"));
    WalkAnimation = LoadObject<UAnimationAsset>(nullptr,
        TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd.MF_Unarmed_Walk_Fwd"));
    JogAnimation = LoadObject<UAnimationAsset>(nullptr,
        TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd.MF_Unarmed_Jog_Fwd"));
    if (IdleAnimation)
    {
        GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        GetMesh()->SetAnimation(IdleAnimation);
        GetMesh()->Play(true);
        ActiveLocomotionAnimation = IdleAnimation;
    }
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->MaxWalkSpeed = JogSpeed;
    Movement->MaxFlySpeed = JogSpeed;
    // The generated World Partition blockout has shown inconsistent floor
    // detection in packaged Mac builds. Constrain this ground-based vertical
    // slice to its insertion elevation so the pawn remains controllable and
    // the animation graph never enters its endless falling state.
    Movement->GravityScale = 0.0f;
    Movement->SetPlaneConstraintNormal(FVector::UpVector);
    Movement->SetPlaneConstraintOrigin(FVector(0.0f, 0.0f, GetActorLocation().Z));
    Movement->SetPlaneConstraintEnabled(true);
    Movement->SetMovementMode(MOVE_Flying);
    InitializeStarterWeapon();
    GetWorldTimerManager().SetTimerForNextTick(this, &AEmberCharacter::ForceGameplayInput);
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
        PC->SetIgnoreLookInput(false);
        PC->SetIgnoreMoveInput(false);
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (OnFootMapping) Subsystem->AddMappingContext(OnFootMapping, 0);
        }
    }
}

void AEmberCharacter::ForceGameplayInput()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC && GetWorld()) PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;
    if (PC->GetPawn() != this) PC->Possess(this);
    EnableInput(PC);
    PC->SetPause(false);
    PC->bShowMouseCursor = false;
    PC->SetInputMode(FInputModeGameOnly());
    PC->SetIgnoreLookInput(false);
    PC->SetIgnoreMoveInput(false);
    PC->FlushPressedKeys();
}

void AEmberCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const float Speed = GetVelocity().Size2D();
    UAnimationAsset* Desired = IdleAnimation;
    if (Speed > 420.0f && JogAnimation) Desired = JogAnimation;
    else if (Speed > 5.0f && WalkAnimation) Desired = WalkAnimation;
    if (Desired && Desired != ActiveLocomotionAnimation)
    {
        GetMesh()->SetAnimation(Desired);
        GetMesh()->Play(true);
        ActiveLocomotionAnimation = Desired;
    }

    // Hardware-level fallback for packaged macOS builds using Common UI.
    // Normal action mappings remain active; rate limiting prevents duplicate
    // fire requests when both paths are delivered in the same frame.
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        const bool bAimHeld = PC->IsInputKeyDown(EKeys::RightMouseButton)
            || PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftTrigger) > 0.35f;
        if (bAimHeld != bDirectAimHeld)
        {
            bAimHeld ? AimStarted() : AimCompleted();
            bDirectAimHeld = bAimHeld;
        }

        const bool bFireHeld = PC->IsInputKeyDown(EKeys::LeftMouseButton)
            || PC->GetInputAnalogKeyState(EKeys::Gamepad_RightTrigger) > 0.35f;
        if (bFireHeld && !bDirectFireHeld) FireStarted();
        else if (!bFireHeld && bDirectFireHeld) FireCompleted();
        bDirectFireHeld = bFireHeld;

        if (PC->WasInputKeyJustPressed(EKeys::R)
            || PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Left)) Reload();
        if (PC->WasInputKeyJustPressed(EKeys::One)) SelectWeapon1();
        if (PC->WasInputKeyJustPressed(EKeys::Two)) SelectWeapon2();
        if (PC->WasInputKeyJustPressed(EKeys::Three)) SelectWeapon3();
        if (PC->WasInputKeyJustPressed(EKeys::Four)) SelectWeapon4();
        if (PC->WasInputKeyJustPressed(EKeys::Five)) SelectWeapon5();
        if (PC->WasInputKeyJustPressed(EKeys::Six)) SelectWeapon6();
    }
}

void AEmberCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction) Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEmberCharacter::Move);
        if (LookAction) Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEmberCharacter::Look);
        if (AimAction)
        {
            Input->BindAction(AimAction, ETriggerEvent::Started, this, &AEmberCharacter::AimStarted);
            Input->BindAction(AimAction, ETriggerEvent::Completed, this, &AEmberCharacter::AimCompleted);
        }
        if (FireAction) Input->BindAction(FireAction, ETriggerEvent::Started, this, &AEmberCharacter::FireStarted);
        if (ReloadAction) Input->BindAction(ReloadAction, ETriggerEvent::Started, this, &AEmberCharacter::Reload);
        if (ShoulderSwapAction) Input->BindAction(ShoulderSwapAction, ETriggerEvent::Started, this, &AEmberCharacter::SwapShoulder);
    }

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AEmberCharacter::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AEmberCharacter::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AEmberCharacter::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AEmberCharacter::LookUp);
    PlayerInputComponent->BindAxis(TEXT("AimAxis"), this, &AEmberCharacter::AimAxis);
    PlayerInputComponent->BindAxis(TEXT("FireAxis"), this, &AEmberCharacter::FireAxis);
    PlayerInputComponent->BindAxis(TEXT("ReloadAxis"), this, &AEmberCharacter::ReloadAxis);
    PlayerInputComponent->BindAxis(TEXT("Weapon1Axis"), this, &AEmberCharacter::Weapon1Axis);
    PlayerInputComponent->BindAxis(TEXT("Weapon2Axis"), this, &AEmberCharacter::Weapon2Axis);
    PlayerInputComponent->BindAxis(TEXT("Weapon3Axis"), this, &AEmberCharacter::Weapon3Axis);
    PlayerInputComponent->BindAxis(TEXT("Weapon4Axis"), this, &AEmberCharacter::Weapon4Axis);
    PlayerInputComponent->BindAxis(TEXT("Weapon5Axis"), this, &AEmberCharacter::Weapon5Axis);
    PlayerInputComponent->BindAxis(TEXT("Weapon6Axis"), this, &AEmberCharacter::Weapon6Axis);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction(TEXT("Aim"), IE_Pressed, this, &AEmberCharacter::AimStarted);
    PlayerInputComponent->BindAction(TEXT("Aim"), IE_Released, this, &AEmberCharacter::AimCompleted);
    PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &AEmberCharacter::FireStarted);
    PlayerInputComponent->BindAction(TEXT("Fire"), IE_Released, this, &AEmberCharacter::FireCompleted);
    PlayerInputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &AEmberCharacter::Reload);
    PlayerInputComponent->BindAction(TEXT("ShoulderSwap"), IE_Pressed, this, &AEmberCharacter::SwapShoulder);
    PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &AEmberCharacter::StartSprint);
    PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &AEmberCharacter::StopSprint);
    PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &AEmberCharacter::ToggleCrouch);
    PlayerInputComponent->BindAction(TEXT("PauseMenu"), IE_Pressed, this, &AEmberCharacter::TogglePauseMenu).bExecuteWhenPaused = true;
    PlayerInputComponent->BindAction(TEXT("Weapon1"), IE_Pressed, this, &AEmberCharacter::SelectWeapon1);
    PlayerInputComponent->BindAction(TEXT("Weapon2"), IE_Pressed, this, &AEmberCharacter::SelectWeapon2);
    PlayerInputComponent->BindAction(TEXT("Weapon3"), IE_Pressed, this, &AEmberCharacter::SelectWeapon3);
    PlayerInputComponent->BindAction(TEXT("Weapon4"), IE_Pressed, this, &AEmberCharacter::SelectWeapon4);
    PlayerInputComponent->BindAction(TEXT("Weapon5"), IE_Pressed, this, &AEmberCharacter::SelectWeapon5);
    PlayerInputComponent->BindAction(TEXT("Weapon6"), IE_Pressed, this, &AEmberCharacter::SelectWeapon6);
}

void AEmberCharacter::Move(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    const FRotator ControlRotation = GetControlRotation();
    const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Axis.Y);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Axis.X);
}

void AEmberCharacter::Look(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    AddControllerYawInput(Axis.X);
    AddControllerPitchInput(Axis.Y);
}

void AEmberCharacter::MoveForward(float Value)
{
    if (FMath::IsNearlyZero(Value)) return;
    const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
}

void AEmberCharacter::MoveRight(float Value)
{
    if (FMath::IsNearlyZero(Value)) return;
    const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
}

void AEmberCharacter::Turn(float Value) { AddControllerYawInput(Value); }
void AEmberCharacter::LookUp(float Value) { AddControllerPitchInput(Value); }
void AEmberCharacter::AimAxis(float Value)
{
    const bool bHeld = Value > 0.35f;
    if (bHeld != bAxisAimHeld) bHeld ? AimStarted() : AimCompleted();
    bAxisAimHeld = bHeld;
}
void AEmberCharacter::FireAxis(float Value)
{
    const bool bHeld = Value > 0.35f;
    if (bHeld && !bAxisFireHeld) FireStarted();
    else if (!bHeld && bAxisFireHeld) FireCompleted();
    bAxisFireHeld = bHeld;
}
void AEmberCharacter::ReloadAxis(float Value)
{
    const bool bHeld = Value > 0.35f;
    if (bHeld && !bAxisReloadHeld) Reload();
    bAxisReloadHeld = bHeld;
}
void AEmberCharacter::HandleWeaponAxis(float Value, int32 Index)
{
    const bool bHeld = Value > 0.35f;
    if (bHeld && !bWeaponAxisHeld[Index]) EquipWeaponIndex(Index);
    bWeaponAxisHeld[Index] = bHeld;
}
void AEmberCharacter::Weapon1Axis(float Value) { HandleWeaponAxis(Value, 0); }
void AEmberCharacter::Weapon2Axis(float Value) { HandleWeaponAxis(Value, 1); }
void AEmberCharacter::Weapon3Axis(float Value) { HandleWeaponAxis(Value, 2); }
void AEmberCharacter::Weapon4Axis(float Value) { HandleWeaponAxis(Value, 3); }
void AEmberCharacter::Weapon5Axis(float Value) { HandleWeaponAxis(Value, 4); }
void AEmberCharacter::Weapon6Axis(float Value) { HandleWeaponAxis(Value, 5); }
void AEmberCharacter::StartSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    GetCharacterMovement()->MaxFlySpeed = SprintSpeed;
}
void AEmberCharacter::StopSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = JogSpeed;
    GetCharacterMovement()->MaxFlySpeed = JogSpeed;
}
void AEmberCharacter::ToggleCrouch() { bIsCrouched ? UnCrouch() : Crouch(); }

void AEmberCharacter::InitializeStarterWeapon()
{
    if (!Weapon) return;
    UEmberWeaponDefinition* Starter = LoadObject<UEmberWeaponDefinition>(nullptr,
        TEXT("/Game/Ember/Weapons/DA_Weapon_AshlineA4.DA_Weapon_AshlineA4"));
    if (!Starter)
    {
        const FPrimaryAssetId StarterId(TEXT("EmberWeapon"), TEXT("Weapon.AshlineA4"));
        const FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(StarterId);
        Starter = Cast<UEmberWeaponDefinition>(AssetPath.TryLoad());
    }
    if (!Starter)
    {
        Starter = NewObject<UEmberWeaponDefinition>(this, TEXT("FallbackStarterWeapon"));
        Starter->Identifier = TEXT("Weapon.AshlineA4.Runtime");
        Starter->DisplayName = FText::FromString(TEXT("ASHLINE A4"));
        Starter->SupportedFireModes = { EEmberFireMode::SemiAutomatic, EEmberFireMode::FullyAutomatic };
    }
    Weapon->InitializeWeapon(Starter, 180);
}

void AEmberCharacter::SetAiming(bool bNewAiming)
{
    bAiming = bNewAiming;
    CameraBoom->TargetArmLength = bAiming ? AimArmLength : ExplorationArmLength;
    CameraBoom->SocketOffset.Z = bAiming ? 58.0f : 70.0f;
    bUseControllerRotationYaw = bAiming;
    GetCharacterMovement()->bOrientRotationToMovement = !bAiming;
}

void AEmberCharacter::AimStarted() { SetAiming(true); }
void AEmberCharacter::AimCompleted() { SetAiming(false); }
void AEmberCharacter::FireStarted()
{
    if (!Weapon) return;
    if (Weapon->RequestFire(BuildShotRequest())) PlayGunshotFeedback();
    if (Weapon->IsAutomatic())
        GetWorldTimerManager().SetTimer(AutomaticFireTimer, [this]() {
            if (Weapon && Weapon->RequestFire(BuildShotRequest())) PlayGunshotFeedback();
        }, 0.06f, true);
}
void AEmberCharacter::FireCompleted()
{
    GetWorldTimerManager().ClearTimer(AutomaticFireTimer);
    if (Weapon) Weapon->StopFire();
}
void AEmberCharacter::Reload() { if (Weapon) Weapon->BeginReload(); }

void AEmberCharacter::PlayGunshotFeedback()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->AddPitchInput(-0.65f);
        PC->AddYawInput(FMath::FRandRange(-0.18f, 0.18f));
    }
    if (MuzzleFlashLight)
    {
        MuzzleFlashLight->SetIntensity(8500.0f);
        GetWorldTimerManager().ClearTimer(MuzzleFlashTimer);
        GetWorldTimerManager().SetTimer(MuzzleFlashTimer, this,
            &AEmberCharacter::ResetMuzzleFlash, 0.045f, false);
    }

    USoundWaveProcedural* Shot = NewObject<USoundWaveProcedural>(this);
    Shot->SetSampleRate(22050);
    Shot->NumChannels = 1;
    Shot->Duration = 0.12f;
    Shot->SoundGroup = SOUNDGROUP_Effects;
    constexpr int32 SampleCount = 2646;
    TArray<int16> Samples;
    Samples.SetNumUninitialized(SampleCount);
    for (int32 Index = 0; Index < SampleCount; ++Index)
    {
        const float T = static_cast<float>(Index) / 22050.0f;
        const float Envelope = FMath::Exp(-T * 34.0f);
        const float Crack = FMath::FRandRange(-1.0f, 1.0f);
        const float Boom = FMath::Sin(2.0f * PI * 92.0f * T);
        Samples[Index] = static_cast<int16>(FMath::Clamp((Crack * 0.72f + Boom * 0.28f) * Envelope, -1.0f, 1.0f) * 28000.0f);
    }
    Shot->QueueAudio(reinterpret_cast<const uint8*>(Samples.GetData()), Samples.Num() * sizeof(int16));
    UGameplayStatics::PlaySoundAtLocation(this, Shot,
        WeaponBarrelVisual ? WeaponBarrelVisual->GetComponentLocation() : GetActorLocation(), 1.0f, 1.0f);
}

void AEmberCharacter::ResetMuzzleFlash()
{
    if (MuzzleFlashLight) MuzzleFlashLight->SetIntensity(0.0f);
}

void AEmberCharacter::TogglePauseMenu()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        const bool bPause = !UGameplayStatics::IsGamePaused(this);
        PC->SetPause(bPause);
        PC->bShowMouseCursor = bPause;
        bPause ? PC->SetInputMode(FInputModeGameAndUI()) : PC->SetInputMode(FInputModeGameOnly());
    }
}

void AEmberCharacter::SelectWeapon1() { EquipWeaponIndex(0); }
void AEmberCharacter::SelectWeapon2() { EquipWeaponIndex(1); }
void AEmberCharacter::SelectWeapon3() { EquipWeaponIndex(2); }
void AEmberCharacter::SelectWeapon4() { EquipWeaponIndex(3); }
void AEmberCharacter::SelectWeapon5() { EquipWeaponIndex(4); }
void AEmberCharacter::SelectWeapon6() { EquipWeaponIndex(5); }

void AEmberCharacter::EquipWeaponIndex(int32 Index)
{
    static const TCHAR* Assets[] = {
        TEXT("DA_Weapon_AshlineA4"), TEXT("DA_Weapon_SparrowC9"), TEXT("DA_Weapon_BreachP12"),
        TEXT("DA_Weapon_VigilD3"), TEXT("DA_Weapon_ForgeL5"), TEXT("DA_Weapon_HarborS9")
    };
    if (!Weapon || Index < 0 || Index >= UE_ARRAY_COUNT(Assets)) return;
    const FString Path = FString::Printf(TEXT("/Game/Ember/Weapons/%s.%s"), Assets[Index], Assets[Index]);
    UEmberWeaponDefinition* Definition = LoadObject<UEmberWeaponDefinition>(nullptr, *Path);
    if (!Definition)
    {
        static const TCHAR* Names[] = {
            TEXT("ASHLINE A4"), TEXT("SPARROW C9"), TEXT("BREACH P12"),
            TEXT("VIGIL D3"), TEXT("FORGE L5"), TEXT("HARBOR S9")
        };
        Definition = NewObject<UEmberWeaponDefinition>(this);
        Definition->Identifier = FName(*FString::Printf(TEXT("Weapon.Runtime.%d"), Index));
        Definition->DisplayName = FText::FromString(Names[Index]);
        Definition->MagazineCapacity = Index == 2 ? 8 : (Index == 4 ? 50 : 30);
        Definition->RoundsPerMinute = Index == 2 ? 180.0f : (Index == 4 ? 750.0f : 600.0f);
        Definition->BaseDamage = Index == 2 ? 75.0f : (Index == 3 ? 55.0f : 30.0f);
        Definition->SupportedFireModes = Index == 2 || Index == 3
            ? TArray<EEmberFireMode>{ EEmberFireMode::SemiAutomatic }
            : TArray<EEmberFireMode>{ EEmberFireMode::FullyAutomatic };
    }
    Weapon->InitializeWeapon(Definition, 180);
}

void AEmberCharacter::SwapShoulder()
{
    bRightShoulder = !bRightShoulder;
    CameraBoom->SocketOffset.Y = bRightShoulder ? ShoulderOffset : -ShoulderOffset;
}

FEmberShotRequest AEmberCharacter::BuildShotRequest() const
{
    FEmberShotRequest Request;
    Request.CameraOrigin = FollowCamera->GetComponentLocation();
    Request.DesiredDirection = FollowCamera->GetForwardVector();
    Request.MuzzleOrigin = WeaponBarrelVisual
        ? WeaponBarrelVisual->GetComponentLocation()
        : GetActorLocation() + GetActorForwardVector() * 50.0f;
    Request.MaximumRange = Weapon ? Weapon->GetMaximumRange() : 10000.0f;
    return Request;
}

FEmberDamageResult AEmberCharacter::ReceiveEmberDamage_Implementation(const FEmberDamageSpec& DamageSpec)
{
    return DamageReceiver ? DamageReceiver->ApplyDamageSpec(DamageSpec) : FEmberDamageResult();
}
