#include "EmberVehicleSeatComponent.h"

#include "EmberCharacter.h"
#include "EmberLog.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "PhysicsEngine/BodySetup.h"

UEmberVehicleEngineSoundWave::UEmberVehicleEngineSoundWave(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    NumChannels = 1;
    SetSampleRate(44100);
    Duration = 3600.0f;
    bLooping = true;
    SoundGroup = SOUNDGROUP_Default;
}

void UEmberVehicleEngineSoundWave::SetEngineState(const float InNormalizedRPM, const float InLoad)
{
    NormalizedRPM.Store(FMath::Clamp(InNormalizedRPM, 0.0f, 1.0f));
    EngineLoad.Store(FMath::Clamp(InLoad, 0.0f, 1.0f));
}

int32 UEmberVehicleEngineSoundWave::OnGeneratePCMAudio(TArray<uint8>& OutAudio, const int32 NumSamples)
{
    if (NumSamples <= 0) return 0;
    const float RPM = NormalizedRPM.Load();
    const float Load = EngineLoad.Load();
    const double Frequency = 42.0 + RPM * 138.0;
    const double PhaseStep = UE_TWO_PI * Frequency / 44100.0;
    const double SubStep = UE_TWO_PI * (Frequency * 0.5) / 44100.0;
    OutAudio.SetNumUninitialized(NumSamples * static_cast<int32>(sizeof(int16)));
    int16* PCM = reinterpret_cast<int16*>(OutAudio.GetData());
    for (int32 Index = 0; Index < NumSamples; ++Index)
    {
        const double Combustion = FMath::Sin(Phase)
            + 0.36 * FMath::Sin(Phase * 2.01)
            + 0.16 * FMath::Sin(Phase * 3.97);
        const double LowBody = FMath::Sin(SubPhase) * (0.42 + Load * 0.24);
        const double Pulse = FMath::Sin(Phase * 0.5) > 0.55 ? 0.09 * Load : 0.0;
        const double Sample = FMath::Clamp((Combustion * 0.22 + LowBody * 0.32 + Pulse)
            * (0.36 + RPM * 0.34), -0.92, 0.92);
        PCM[Index] = static_cast<int16>(Sample * 32767.0);
        Phase = FMath::Fmod(Phase + PhaseStep, UE_TWO_PI);
        SubPhase = FMath::Fmod(SubPhase + SubStep, UE_TWO_PI);
    }
    return NumSamples;
}

UEmberVehicleSeatComponent::UEmberVehicleSeatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f;
}

void UEmberVehicleSeatComponent::TickComponent(const float DeltaTime, const ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!DriverCharacter.IsValid() || !EngineWave) return;
    const float SpeedRPM = FMath::Clamp(GetSpeedKPH() / 150.0f, 0.0f, 1.0f);
    const float TargetRPM = FMath::Clamp(0.11f + SpeedRPM * 0.72f
        + FMath::Abs(ThrottleInput) * 0.28f, 0.0f, 1.0f);
    EngineWave->SetEngineState(TargetRPM,
        FMath::Max(FMath::Abs(ThrottleInput), BrakeInput * 0.25f));
    if (EngineAudio)
    {
        EngineAudio->SetVolumeMultiplier(0.34f + TargetRPM * 0.34f);
    }
    if (!bMotionLogged && GetSpeedKPH() > 2.0f)
    {
        bMotionLogged = true;
        UE_LOG(LogEmberCombat, Log, TEXT("Vehicle motion confirmed: speed=%.1f KPH"), GetSpeedKPH());
    }
}

bool UEmberVehicleSeatComponent::SeatDriver(AEmberCharacter* InDriver)
{
    APawn* VehiclePawn = Cast<APawn>(GetOwner());
    if (!VehiclePawn || !InDriver || DriverCharacter.IsValid()) return false;
    DriverCharacter = InDriver;
    BuildVisibleDriverPose(InDriver);
    BuildVehicleCamera();
    StartEngineAudio();
    InDriver->PrepareForVehicle(VehiclePawn);
    UE_LOG(LogEmberCombat, Log, TEXT("Vehicle entered: %s"), *VehiclePawn->GetName());
    return true;
}

void UEmberVehicleSeatComponent::BuildVehicleCamera()
{
    APawn* VehiclePawn = Cast<APawn>(GetOwner());
    if (!VehiclePawn || !VehiclePawn->GetRootComponent() || VehicleCameraBoom || VehicleCamera) return;

    // The example vehicle's authored camera is intended for its standalone
    // example map and can resolve beneath a World Partition road after the
    // pawn is dynamically possessed. Ember owns a deterministic third-person
    // chase camera so entering a car never changes the game into a broken or
    // first-person view.
    TArray<UCameraComponent*> ExistingCameras;
    VehiclePawn->GetComponents<UCameraComponent>(ExistingCameras);
    for (UCameraComponent* ExistingCamera : ExistingCameras)
    {
        if (ExistingCamera) ExistingCamera->Deactivate();
    }

    VehicleCameraBoom = NewObject<USpringArmComponent>(VehiclePawn, TEXT("EmberVehicleCameraBoom"));
    VehicleCameraBoom->RegisterComponent();
    VehicleCameraBoom->AttachToComponent(VehiclePawn->GetRootComponent(),
        FAttachmentTransformRules::KeepRelativeTransform);
    VehicleCameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 155.0f));
    CameraYawOffset = 0.0f;
    CameraPitch = -8.0f;
    VehicleCameraBoom->SetRelativeRotation(FRotator(CameraPitch, CameraYawOffset, 0.0f));
    VehicleCameraBoom->TargetArmLength = 640.0f;
    // Keep the orbit local to the car. The example pawn modifies controller
    // rotation during possession; using that value made its camera snap to a
    // steep overhead angle. Ember applies mouse/right-stick deltas directly
    // to this clamped boom instead.
    VehicleCameraBoom->bUsePawnControlRotation = false;
    VehicleCameraBoom->bInheritPitch = false;
    VehicleCameraBoom->bInheritYaw = false;
    VehicleCameraBoom->bInheritRoll = false;
    VehicleCameraBoom->bDoCollisionTest = true;
    VehicleCameraBoom->ProbeSize = 18.0f;
    VehicleCameraBoom->ProbeChannel = ECC_Camera;
    VehicleCameraBoom->bEnableCameraLag = true;
    VehicleCameraBoom->CameraLagSpeed = 8.0f;
    VehicleCameraBoom->CameraLagMaxDistance = 85.0f;
    VehicleCameraBoom->bEnableCameraRotationLag = true;
    VehicleCameraBoom->CameraRotationLagSpeed = 10.0f;

    VehicleCamera = NewObject<UCameraComponent>(VehiclePawn, TEXT("EmberVehicleCamera"));
    VehicleCamera->RegisterComponent();
    VehicleCamera->AttachToComponent(VehicleCameraBoom,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        USpringArmComponent::SocketName);
    VehicleCamera->SetFieldOfView(78.0f);
    VehicleCamera->bUsePawnControlRotation = false;
    VehicleCamera->Activate(true);
}

void UEmberVehicleSeatComponent::AddCameraInput(const float YawDelta, const float PitchDelta)
{
    if (!VehicleCameraBoom) return;
    CameraYawOffset = FMath::UnwindDegrees(CameraYawOffset + YawDelta);
    CameraPitch = FMath::Clamp(CameraPitch + PitchDelta, -32.0f, 8.0f);
    VehicleCameraBoom->SetRelativeRotation(FRotator(CameraPitch, CameraYawOffset, 0.0f));
}

void UEmberVehicleSeatComponent::DestroyVehicleCamera()
{
    APawn* VehiclePawn = Cast<APawn>(GetOwner());
    if (VehicleCamera) VehicleCamera->DestroyComponent();
    if (VehicleCameraBoom) VehicleCameraBoom->DestroyComponent();
    VehicleCamera = nullptr;
    VehicleCameraBoom = nullptr;

    // Restore the asset's original camera for Editor inspection. It remains
    // unused until the next occupant receives a fresh Ember chase camera.
    if (VehiclePawn)
    {
        TArray<UCameraComponent*> ExistingCameras;
        VehiclePawn->GetComponents<UCameraComponent>(ExistingCameras);
        for (UCameraComponent* ExistingCamera : ExistingCameras)
        {
            if (ExistingCamera)
            {
                ExistingCamera->Activate(true);
                break;
            }
        }
    }
}

void UEmberVehicleSeatComponent::BuildVisibleDriverPose(AEmberCharacter* InDriver)
{
    APawn* VehiclePawn = Cast<APawn>(GetOwner());
    USkeletalMeshComponent* SourceMesh = InDriver ? InDriver->GetMesh() : nullptr;
    USkeletalMeshComponent* VehicleMesh = VehiclePawn ? VehiclePawn->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
    if (!VehiclePawn || !SourceMesh || !SourceMesh->GetSkeletalMeshAsset()) return;

    DriverPoseMesh = NewObject<UPoseableMeshComponent>(VehiclePawn, TEXT("EmberVisibleDriverPose"));
    DriverPoseMesh->SetSkinnedAssetAndUpdate(SourceMesh->GetSkeletalMeshAsset(), true);
    DriverPoseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DriverPoseMesh->SetCastShadow(true);
    for (int32 MaterialIndex = 0; MaterialIndex < SourceMesh->GetNumMaterials(); ++MaterialIndex)
    {
        DriverPoseMesh->SetMaterial(MaterialIndex, SourceMesh->GetMaterial(MaterialIndex));
    }
    DriverPoseMesh->RegisterComponent();
    DriverPoseMesh->AttachToComponent(VehicleMesh ? VehicleMesh : VehiclePawn->GetRootComponent(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    DriverPoseMesh->CopyPoseFromSkeletalComponent(SourceMesh);
    // The car uses X-forward/Y-right. Manny's root is at the feet, so the
    // negative Z placement puts the pelvis on the left seat while the folded
    // leg pose remains inside the cabin and the head/shoulders remain visible.
    DriverPoseMesh->SetRelativeLocation(FVector(-18.0f, -43.0f, -48.0f));
    DriverPoseMesh->SetRelativeRotation(SourceMesh->GetRelativeRotation());
    DriverPoseMesh->SetRelativeScale3D(FVector(0.92f));

    const auto RotateLocal = [this](const FName Bone, const FRotator Delta)
    {
        if (!DriverPoseMesh || DriverPoseMesh->GetBoneIndex(Bone) == INDEX_NONE) return;
        FTransform BoneTransform = DriverPoseMesh->GetBoneTransformByName(Bone, EBoneSpaces::ComponentSpace);
        BoneTransform.ConcatenateRotation(Delta.Quaternion());
        DriverPoseMesh->SetBoneTransformByName(Bone, BoneTransform, EBoneSpaces::ComponentSpace);
    };
    RotateLocal(TEXT("thigh_l"), FRotator(0.0f, 0.0f, 72.0f));
    RotateLocal(TEXT("thigh_r"), FRotator(0.0f, 0.0f, 72.0f));
    RotateLocal(TEXT("calf_l"), FRotator(0.0f, 0.0f, -92.0f));
    RotateLocal(TEXT("calf_r"), FRotator(0.0f, 0.0f, -92.0f));
    RotateLocal(TEXT("spine_01"), FRotator(0.0f, 0.0f, -8.0f));
}

void UEmberVehicleSeatComponent::StartEngineAudio()
{
    APawn* VehiclePawn = Cast<APawn>(GetOwner());
    if (!VehiclePawn || EngineAudio) return;
    EngineWave = NewObject<UEmberVehicleEngineSoundWave>(this, TEXT("EmberVehicleEngineWave"));
    EngineAudio = NewObject<UAudioComponent>(VehiclePawn, TEXT("EmberVehicleEngineAudio"));
    EngineAudio->bAutoActivate = false;
    EngineAudio->bAllowSpatialization = true;
    EngineAudio->SetVolumeMultiplier(0.42f);
    EngineAudio->SetSound(EngineWave);
    EngineAudio->RegisterComponent();
    EngineAudio->AttachToComponent(VehiclePawn->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    EngineAudio->Play();
}

float UEmberVehicleSeatComponent::GetSpeedKPH() const
{
    const AActor* Vehicle = GetOwner();
    return Vehicle ? Vehicle->GetVelocity().Size() * 0.036f : 0.0f;
}

float UEmberVehicleSeatComponent::GetForwardSpeedKPH() const
{
    const AActor* Vehicle = GetOwner();
    return Vehicle ? FVector::DotProduct(Vehicle->GetVelocity(), Vehicle->GetActorForwardVector()) * 0.036f : 0.0f;
}

void UEmberVehicleSeatComponent::SetControlState(const float InThrottle, const float InSteering,
    const float InBrake, const bool bInHandbrake)
{
    ThrottleInput = FMath::Clamp(InThrottle, -1.0f, 1.0f);
    SteeringInput = FMath::Clamp(InSteering, -1.0f, 1.0f);
    BrakeInput = FMath::Clamp(InBrake, 0.0f, 1.0f);
    bHandbrake = bInHandbrake;
}

FVector UEmberVehicleSeatComponent::FindSafeExitLocation(AEmberCharacter* Driver) const
{
    const AActor* Vehicle = GetOwner();
    if (!Vehicle || !Driver || !GetWorld()) return Vehicle ? Vehicle->GetActorLocation() : FVector::ZeroVector;
    const float Radius = Driver->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const float HalfHeight = Driver->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const FVector LocalCandidates[] = {
        FVector(15.0f, -245.0f, HalfHeight + 18.0f),
        FVector(15.0f, 245.0f, HalfHeight + 18.0f),
        FVector(-275.0f, 0.0f, HalfHeight + 18.0f),
        FVector(285.0f, 0.0f, HalfHeight + 18.0f),
    };
    FCollisionQueryParams Params(SCENE_QUERY_STAT(EmberVehicleExit), false, Vehicle);
    Params.AddIgnoredActor(Driver);
    for (const FVector& Local : LocalCandidates)
    {
        const FVector Candidate = Vehicle->GetActorTransform().TransformPosition(Local);
        if (!GetWorld()->OverlapBlockingTestByChannel(Candidate, FQuat::Identity, ECC_Pawn,
            FCollisionShape::MakeCapsule(Radius, HalfHeight), Params))
        {
            return Candidate;
        }
    }
    return Vehicle->GetActorLocation() + FVector(0.0f, 0.0f, HalfHeight + 220.0f);
}

AEmberCharacter* UEmberVehicleSeatComponent::ReleaseDriver(FVector& OutExitLocation, FRotator& OutExitRotation)
{
    AEmberCharacter* Driver = DriverCharacter.Get();
    const AActor* Vehicle = GetOwner();
    if (!Driver || !Vehicle) return nullptr;
    OutExitLocation = FindSafeExitLocation(Driver);
    OutExitRotation = FRotator(0.0f, Vehicle->GetActorRotation().Yaw, 0.0f);
    if (EngineAudio) EngineAudio->Stop();
    if (DriverPoseMesh) DriverPoseMesh->DestroyComponent();
    DestroyVehicleCamera();
    DriverPoseMesh = nullptr;
    EngineAudio = nullptr;
    EngineWave = nullptr;
    DriverCharacter.Reset();
    bMotionLogged = false;
    SetControlState(0.0f, 0.0f, 1.0f, true);
    UE_LOG(LogEmberCombat, Log, TEXT("Vehicle exited: %s"), *Vehicle->GetName());
    return Driver;
}
