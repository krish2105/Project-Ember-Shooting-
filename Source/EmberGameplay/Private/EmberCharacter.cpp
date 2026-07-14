#include "EmberCharacter.h"
#include "Camera/CameraComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "EmberArmorComponent.h"
#include "EmberDamageReceiverComponent.h"
#include "EmberHealthComponent.h"
#include "EmberInteractionComponent.h"
#include "EmberInventoryComponent.h"
#include "EmberLog.h"
#include "EmberWeaponComponent.h"
#include "EmberWeaponDefinition.h"
#include "Engine/AssetManager.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Sound/SoundWaveProcedural.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    UEmberWeaponDefinition* LoadWeaponDefinition(const FName Identifier, const TCHAR* AssetName)
    {
        const FPrimaryAssetId AssetId(TEXT("EmberWeapon"), Identifier);
        const FSoftObjectPath ManagedPath = UAssetManager::Get().GetPrimaryAssetPath(AssetId);
        if (UEmberWeaponDefinition* Managed = Cast<UEmberWeaponDefinition>(ManagedPath.TryLoad()))
        {
            return Managed;
        }
        const FString DirectPath = FString::Printf(
            TEXT("/Game/Ember/Weapons/%s.%s"), AssetName, AssetName);
        return LoadObject<UEmberWeaponDefinition>(nullptr, *DirectPath);
    }
}

AEmberCharacter::AEmberCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->GravityScale = 1.0f;
    GetCharacterMovement()->MaxWalkSpeed = JogSpeed;
    GetCharacterMovement()->BrakingDecelerationWalking = 1600.0f;
    GetCharacterMovement()->bUseSeparateBrakingFriction = true;

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
    // The Shooter Variant animation rig authors the weapon alignment around
    // HandGrip_R. Attaching to the raw hand_r bone leaves the rifle ninety
    // degrees across the torso because that bone has no weapon-facing socket
    // basis.
    WeaponBodyVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponBodyVisual"));
    WeaponBodyVisual->SetupAttachment(GetMesh(), TEXT("HandGrip_R"));
    WeaponBodyVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponBodyVisual->SetRelativeLocation(FVector::ZeroVector);
    WeaponBodyVisual->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
    WeaponBodyVisual->SetRelativeScale3D(FVector::OneVector);

    // A hidden cube is retained only as a lightweight transient tracer mesh.
    WeaponBarrelVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponBarrelVisual"));
    WeaponBarrelVisual->SetupAttachment(RootComponent);
    WeaponBarrelVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponBarrelVisual->SetVisibility(false);
    WeaponBarrelVisual->SetHiddenInGame(true);
    if (CubeMesh.Succeeded())
    {
        WeaponBarrelVisual->SetStaticMesh(CubeMesh.Object);
    }
    ShotTracerVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShotTracerVisual"));
    ShotTracerVisual->SetupAttachment(RootComponent);
    ShotTracerVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ShotTracerVisual->SetCastShadow(false);
    ShotTracerVisual->SetVisibility(false);
    ShotTracerVisual->SetHiddenInGame(true);
    if (CubeMesh.Succeeded())
    {
        ShotTracerVisual->SetStaticMesh(CubeMesh.Object);
    }
    MuzzleFlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("MuzzleFlashLight"));
    MuzzleFlashLight->SetupAttachment(WeaponBodyVisual);
    // The template rifle is authored lengthwise on local +Y. The grip socket
    // rotates that axis into character-forward, so the muzzle belongs at the
    // positive-Y end of the mesh rather than on the old primitive's X axis.
    MuzzleFlashLight->SetRelativeLocation(FVector(0.0f, 58.0f, 7.0f));
    MuzzleFlashLight->SetAttenuationRadius(450.0f);
    MuzzleFlashLight->SetLightColor(FLinearColor(1.0f, 0.35f, 0.04f));
    MuzzleFlashLight->SetIntensity(0.0f);

    ImpactFeedbackLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ImpactFeedbackLight"));
    ImpactFeedbackLight->SetupAttachment(RootComponent);
    ImpactFeedbackLight->SetAttenuationRadius(240.0f);
    ImpactFeedbackLight->SetIntensity(0.0f);
    ImpactFeedbackLight->SetVisibility(false);

    GunshotAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("GunshotAudio"));
    GunshotAudio->SetupAttachment(RootComponent);
    GunshotAudio->bAutoActivate = false;
}

void AEmberCharacter::BeginPlay()
{
    Super::BeginPlay();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->MaxWalkSpeed = JogSpeed;
    Movement->GravityScale = 1.0f;
    Movement->SetPlaneConstraintEnabled(false);
    Movement->SetMovementMode(MOVE_Walking);
    InitializeStarterWeapon();
    InitializeGunshotAudio();
    Weapon->OnShotResolved.AddDynamic(this, &AEmberCharacter::HandleShotResolved);
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        // A packaged relaunch must not inherit a stale extreme look pitch from
        // the previous session/window capture. Start every mission from the
        // authored third-person horizon and then hand control back to input.
        PC->SetControlRotation(FRotator(0.0f, GetActorRotation().Yaw, 0.0f));
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
        PC->SetIgnoreMoveInput(false);
    }
}

void AEmberCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!CameraBoom || !FollowCamera) return;

    const float TargetArmLength = bAiming ? AimArmLength : ExplorationArmLength;
    const float TargetHeight = bAiming ? 58.0f : 70.0f;
    const float TargetShoulder = bRightShoulder ? ShoulderOffset : -ShoulderOffset;
    CameraBoom->TargetArmLength = FMath::FInterpTo(
        CameraBoom->TargetArmLength, TargetArmLength, DeltaSeconds, CameraBlendSpeed);
    FVector Offset = CameraBoom->SocketOffset;
    Offset.Y = FMath::FInterpTo(Offset.Y, TargetShoulder, DeltaSeconds, CameraBlendSpeed);
    Offset.Z = FMath::FInterpTo(Offset.Z, TargetHeight, DeltaSeconds, CameraBlendSpeed);
    CameraBoom->SocketOffset = Offset;
    FollowCamera->SetFieldOfView(FMath::FInterpTo(
        FollowCamera->FieldOfView, bAiming ? 70.0f : 86.0f, DeltaSeconds, CameraBlendSpeed));

    WeaponVisualRecoil = FMath::FInterpTo(WeaponVisualRecoil, 0.0f, DeltaSeconds, 15.0f);
    if (WeaponBodyVisual)
    {
        const float ReloadProgress = Weapon ? Weapon->GetReloadProgress() : 0.0f;
        const float ReloadPose = FMath::Sin(ReloadProgress * PI);
        FVector Location = ActiveWeaponPresentationTransform.GetLocation();
        FRotator Rotation = ActiveWeaponPresentationTransform.Rotator();
        Location.X -= WeaponVisualRecoil * 4.0f;
        Rotation.Pitch += WeaponVisualRecoil * 2.5f;
        Location.Z -= ReloadPose * 13.0f;
        Rotation.Pitch -= ReloadPose * 12.0f;
        Rotation.Roll += ReloadPose * 32.0f;
        WeaponBodyVisual->SetRelativeLocationAndRotation(Location, Rotation);
    }
}

void AEmberCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AEmberCharacter::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AEmberCharacter::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AEmberCharacter::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AEmberCharacter::LookUp);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction(TEXT("Aim"), IE_Pressed, this, &AEmberCharacter::AimStarted);
    PlayerInputComponent->BindAction(TEXT("Aim"), IE_Released, this, &AEmberCharacter::AimCompleted);
    PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &AEmberCharacter::FireStarted);
    PlayerInputComponent->BindAction(TEXT("Fire"), IE_Released, this, &AEmberCharacter::FireCompleted);
    PlayerInputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &AEmberCharacter::Reload);
    PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AEmberCharacter::Interact);
    PlayerInputComponent->BindAction(TEXT("Melee"), IE_Pressed, this, &AEmberCharacter::MeleeAttack);
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
    PlayerInputComponent->BindAction(TEXT("WeaponNext"), IE_Pressed, this, &AEmberCharacter::CycleWeaponNext);
    PlayerInputComponent->BindAction(TEXT("WeaponPrevious"), IE_Pressed, this, &AEmberCharacter::CycleWeaponPrevious);
    PlayerInputComponent->BindAction(TEXT("TacticalOverlay"), IE_Pressed, this, &AEmberCharacter::ToggleTacticalOverlay).bExecuteWhenPaused = true;
    PlayerInputComponent->BindAction(TEXT("ControlsOverlay"), IE_Pressed, this, &AEmberCharacter::ToggleControlsOverlay).bExecuteWhenPaused = true;
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

void AEmberCharacter::Turn(float Value)
{
    if (FMath::IsNearlyZero(Value)) return;
    AddControllerYawInput(Value);
    if (!bLookInputConfirmed)
    {
        bLookInputConfirmed = true;
        UE_LOG(LogEmberCombat, Log, TEXT("Player look input confirmed: yaw delta=%.4f"), Value);
    }
}

void AEmberCharacter::LookUp(float Value)
{
    if (FMath::IsNearlyZero(Value)) return;
    AddControllerPitchInput(Value);
    if (!bLookInputConfirmed)
    {
        bLookInputConfirmed = true;
        UE_LOG(LogEmberCombat, Log, TEXT("Player look input confirmed: pitch delta=%.4f"), Value);
    }
}
void AEmberCharacter::StartSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}
void AEmberCharacter::StopSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = JogSpeed;
}
void AEmberCharacter::ToggleCrouch() { bIsCrouched ? UnCrouch() : Crouch(); }

void AEmberCharacter::InitializeStarterWeapon()
{
    SlotMagazineAmmo.Init(INDEX_NONE, 6);
    SlotReserveAmmo.Init(INDEX_NONE, 6);
    EquipWeaponIndex(0);
}

void AEmberCharacter::SetAiming(bool bNewAiming)
{
    bAiming = bNewAiming;
    bUseControllerRotationYaw = bAiming;
    GetCharacterMovement()->bOrientRotationToMovement = !bAiming;
}

void AEmberCharacter::AimStarted()
{
    SetAiming(bToggleAimInput ? !bAiming : true);
    UE_LOG(LogEmberCombat, Log, TEXT("Player aim started: %s"), bAiming ? TEXT("AIM") : TEXT("HIP"));
}
void AEmberCharacter::AimCompleted()
{
    if (!bToggleAimInput) SetAiming(false);
}
void AEmberCharacter::FireStarted()
{
    if (!Weapon) return;
    if (Weapon->IsReloading() && Weapon->GetMagazineAmmo() > 0) Weapon->CancelReload();
    bFireInputHeld = true;
    TryFire();
    if (Weapon->IsAutomatic())
        GetWorldTimerManager().SetTimer(AutomaticFireTimer, [this]() {
            TryFire();
        }, 0.06f, true);
}
void AEmberCharacter::FireCompleted()
{
    bFireInputHeld = false;
    GetWorldTimerManager().ClearTimer(AutomaticFireTimer);
    if (Weapon) Weapon->StopFire();
}
void AEmberCharacter::Reload()
{
    const bool bStarted = Weapon && Weapon->BeginReload();
    if (bStarted) PlayCurrentWeaponMontage(true);
    UE_LOG(LogEmberCombat, Log, TEXT("Player reload requested: %s"), bStarted ? TEXT("STARTED") : TEXT("REJECTED"));
}

void AEmberCharacter::Interact()
{
    if (Interaction && FollowCamera)
    {
        Interaction->TryInteract(FollowCamera->GetComponentLocation(), FollowCamera->GetForwardVector());
    }
}

void AEmberCharacter::MeleeAttack()
{
    if (!bMeleeReady || !GetWorld() || !FollowCamera) return;
    bMeleeReady = false;
    const FVector Start = FollowCamera->GetComponentLocation();
    const FVector End = Start + FollowCamera->GetForwardVector() * 190.0f;
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(EmberMelee), true, this);
    if (GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility,
        FCollisionShape::MakeSphere(55.0f), Params))
    {
        AActor* Target = Hit.GetActor();
        if (Target && Target->GetClass()->ImplementsInterface(UEmberDamageable::StaticClass()))
        {
            FEmberDamageSpec Damage;
            Damage.BaseDamage = 45.0f;
            Damage.ArmorModifier = 1.1f;
            Damage.Stagger = 45.0f;
            Damage.ShotDirection = FollowCamera->GetForwardVector();
            Damage.ImpactPoint = Hit.ImpactPoint;
            const FEmberDamageResult Result = IEmberDamageable::Execute_ReceiveEmberDamage(Target, Damage);
            if (Result.AppliedToHealth + Result.AppliedToArmor > 0.0f)
                LastHitTimeSeconds = GetWorld()->GetTimeSeconds();
        }
    }
    GetWorldTimerManager().SetTimer(MeleeCooldownTimer, [this]() { bMeleeReady = true; }, 0.65f, false);
}

bool AEmberCharacter::TryFire()
{
    if (!Weapon) return false;
    LastShotRequest = BuildShotRequest();
    if (!Weapon->RequestFire(LastShotRequest)) return false;
    UE_LOG(LogEmberCombat, Log, TEXT("Player weapon fired; magazine=%d reserve=%d"),
        Weapon->GetMagazineAmmo(), Weapon->GetReserveAmmo());
    WeaponVisualRecoil = FMath::Min(2.25f, WeaponVisualRecoil + 1.15f);
    PlayCurrentWeaponMontage(false);
    PlayGunshotFeedback();
    return true;
}

void AEmberCharacter::HandleShotResolved(const FEmberShotResult& Result)
{
    UStaticMesh* TracerMesh = WeaponBarrelVisual ? WeaponBarrelVisual->GetStaticMesh() : nullptr;
    const FVector End = Result.bHit
        ? FVector(Result.ImpactPoint)
        : FVector(LastShotRequest.CameraOrigin) + FVector(LastShotRequest.DesiredDirection) * 7000.0f;
    const FVector Start = FVector(LastShotRequest.MuzzleOrigin);
    const FVector Delta = End - Start;
    const float Distance = Delta.Size();
    if (TracerMesh && ShotTracerVisual && Distance > 1.0f)
    {
        ShotTracerVisual->SetStaticMesh(TracerMesh);
        ShotTracerVisual->SetWorldLocation(Start + Delta * 0.5f);
        ShotTracerVisual->SetWorldRotation(Delta.Rotation());
        ShotTracerVisual->SetWorldScale3D(FVector(Distance / 100.0f, 0.012f, 0.012f));
        ShotTracerVisual->SetHiddenInGame(false);
        ShotTracerVisual->SetVisibility(true);
        GetWorldTimerManager().ClearTimer(ShotTracerTimer);
        GetWorldTimerManager().SetTimer(ShotTracerTimer, this,
            &AEmberCharacter::ResetShotTracer, 0.055f, false);
    }
    if (Result.bHit && ImpactFeedbackLight)
    {
        if (Result.bDamagedActor) LastHitTimeSeconds = GetWorld()->GetTimeSeconds();
        ImpactFeedbackLight->SetLightColor(Result.HitActor && Result.HitActor->ActorHasTag(TEXT("EmberEnemy"))
            ? FLinearColor(1.0f, 0.05f, 0.02f) : FLinearColor(1.0f, 0.55f, 0.08f));
        ImpactFeedbackLight->SetIntensity(5000.0f);
        ImpactFeedbackLight->SetWorldLocation(FVector(Result.ImpactPoint) + FVector(Result.ImpactNormal) * 3.0f);
        ImpactFeedbackLight->SetVisibility(true);
        GetWorldTimerManager().ClearTimer(ImpactFeedbackTimer);
        GetWorldTimerManager().SetTimer(ImpactFeedbackTimer, this,
            &AEmberCharacter::ResetImpactFeedback, 0.09f, false);
    }
}

bool AEmberCharacter::HasHostileUnderCrosshair() const
{
    if (!GetWorld() || !FollowCamera) return false;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(EmberTargetCheck), true, this);
    FHitResult Hit;
    const FVector Start = FollowCamera->GetComponentLocation();
    const FVector End = Start + FollowCamera->GetForwardVector() * 30000.0f;
    return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params)
        && Hit.GetActor() && Hit.GetActor()->ActorHasTag(TEXT("EmberEnemy"));
}

bool AEmberCharacter::ShouldShowHitMarker() const
{
    return GetWorld() && GetWorld()->GetTimeSeconds() - LastHitTimeSeconds < 0.18;
}

float AEmberCharacter::GetDamageFeedbackAlpha() const
{
    if (!GetWorld()) return 0.0f;
    return 1.0f - FMath::Clamp(static_cast<float>((GetWorld()->GetTimeSeconds() - LastDamageTimeSeconds) / 0.7), 0.0f, 1.0f);
}

void AEmberCharacter::WriteWeaponCheckpoint(FEmberCheckpointSnapshot& Snapshot) const
{
    for (int32 Index = 0; Index < 6; ++Index)
    {
        int32 Magazine = SlotMagazineAmmo.IsValidIndex(Index) ? SlotMagazineAmmo[Index] : INDEX_NONE;
        int32 Reserve = SlotReserveAmmo.IsValidIndex(Index) ? SlotReserveAmmo[Index] : INDEX_NONE;
        if (Index == CurrentWeaponIndex && Weapon)
        {
            Magazine = Weapon->GetMagazineAmmo();
            Reserve = Weapon->GetReserveAmmo();
        }
        if (Magazine != INDEX_NONE)
        {
            Snapshot.AmmunitionByType.Add(
                FName(*FString::Printf(TEXT("Slot%d.Magazine"), Index + 1)), Magazine);
            Snapshot.AmmunitionByType.Add(
                FName(*FString::Printf(TEXT("Slot%d.Reserve"), Index + 1)), Reserve);
        }
    }
    Snapshot.AmmunitionByType.Add(TEXT("CurrentSlot"), CurrentWeaponIndex);
}

bool AEmberCharacter::RestoreWeaponCheckpoint(const FEmberCheckpointSnapshot& Snapshot)
{
    const int32* SavedCurrent = Snapshot.AmmunitionByType.Find(TEXT("CurrentSlot"));
    if (!SavedCurrent || *SavedCurrent < 0 || *SavedCurrent >= 6) return false;
    SlotMagazineAmmo.Init(INDEX_NONE, 6);
    SlotReserveAmmo.Init(INDEX_NONE, 6);
    for (int32 Index = 0; Index < 6; ++Index)
    {
        const int32* Magazine = Snapshot.AmmunitionByType.Find(
            FName(*FString::Printf(TEXT("Slot%d.Magazine"), Index + 1)));
        const int32* Reserve = Snapshot.AmmunitionByType.Find(
            FName(*FString::Printf(TEXT("Slot%d.Reserve"), Index + 1)));
        if (Magazine && Reserve)
        {
            SlotMagazineAmmo[Index] = FMath::Max(0, *Magazine);
            SlotReserveAmmo[Index] = FMath::Max(0, *Reserve);
        }
    }
    CurrentWeaponIndex = INDEX_NONE;
    EquipWeaponIndex(*SavedCurrent);
    return CurrentWeaponIndex == *SavedCurrent;
}

void AEmberCharacter::PlayGunshotFeedback()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        const float AimMultiplier = bAiming ? 0.78f : 1.0f;
        PC->AddPitchInput(-(Weapon ? Weapon->GetVerticalRecoil() : 0.65f) * 1.35f * AimMultiplier);
        const float Horizontal = Weapon ? Weapon->GetHorizontalRecoil() : 0.18f;
        PC->AddYawInput(FMath::FRandRange(-Horizontal, Horizontal) * 1.2f * AimMultiplier);
    }
    if (MuzzleFlashLight)
    {
        MuzzleFlashLight->SetIntensity(8500.0f);
        GetWorldTimerManager().ClearTimer(MuzzleFlashTimer);
        GetWorldTimerManager().SetTimer(MuzzleFlashTimer, this,
            &AEmberCharacter::ResetMuzzleFlash, 0.045f, false);
    }

    if (GunshotWave && GunshotAudio && !GunshotPCM.IsEmpty())
    {
        GunshotWave->ResetAudio();
        GunshotWave->QueueAudio(GunshotPCM.GetData(), GunshotPCM.Num());
        GunshotAudio->Stop();
        GunshotAudio->Play();
    }
}

void AEmberCharacter::InitializeGunshotAudio()
{
    if (!GunshotAudio || GunshotWave) return;
    GunshotWave = NewObject<USoundWaveProcedural>(this, TEXT("RuntimeGunshotWave"));
    GunshotWave->SetSampleRate(22050);
    GunshotWave->NumChannels = 1;
    GunshotWave->Duration = 0.12f;
    GunshotWave->SoundGroup = SOUNDGROUP_Effects;
    constexpr int32 SampleCount = 2646;
    GunshotPCM.SetNumUninitialized(SampleCount * sizeof(int16));
    int16* Samples = reinterpret_cast<int16*>(GunshotPCM.GetData());
    FRandomStream Noise(0xE4B3);
    for (int32 Index = 0; Index < SampleCount; ++Index)
    {
        const float T = static_cast<float>(Index) / 22050.0f;
        const float Envelope = FMath::Exp(-T * 34.0f);
        const float Crack = Noise.FRandRange(-1.0f, 1.0f);
        const float Boom = FMath::Sin(2.0f * PI * 92.0f * T);
        Samples[Index] = static_cast<int16>(
            FMath::Clamp((Crack * 0.72f + Boom * 0.28f) * Envelope, -1.0f, 1.0f) * 28000.0f);
    }
    GunshotAudio->SetSound(GunshotWave);
}

void AEmberCharacter::ResetMuzzleFlash()
{
    if (MuzzleFlashLight) MuzzleFlashLight->SetIntensity(0.0f);
}

void AEmberCharacter::ResetShotTracer()
{
    if (ShotTracerVisual)
    {
        ShotTracerVisual->SetVisibility(false);
        ShotTracerVisual->SetHiddenInGame(true);
    }
}

void AEmberCharacter::ResetImpactFeedback()
{
    if (ImpactFeedbackLight)
    {
        ImpactFeedbackLight->SetIntensity(0.0f);
        ImpactFeedbackLight->SetVisibility(false);
    }
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

void AEmberCharacter::ToggleTacticalOverlay()
{
    bTacticalOverlayVisible = !bTacticalOverlayVisible;
    if (bTacticalOverlayVisible) bControlsOverlayVisible = false;
}

void AEmberCharacter::ToggleControlsOverlay()
{
    bControlsOverlayVisible = !bControlsOverlayVisible;
    if (bControlsOverlayVisible) bTacticalOverlayVisible = false;
}

void AEmberCharacter::CycleWeaponNext()
{
    EquipWeaponIndex(CurrentWeaponIndex == INDEX_NONE ? 0 : (CurrentWeaponIndex + 1) % 6);
}

void AEmberCharacter::CycleWeaponPrevious()
{
    EquipWeaponIndex(CurrentWeaponIndex == INDEX_NONE ? 0 : (CurrentWeaponIndex + 5) % 6);
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
    static const TCHAR* Identifiers[] = {
        TEXT("Weapon.AshlineA4"), TEXT("Weapon.SparrowC9"), TEXT("Weapon.BreachP12"),
        TEXT("Weapon.VigilD3"), TEXT("Weapon.ForgeL5"), TEXT("Weapon.HarborS9")
    };
    if (!Weapon || Index < 0 || Index >= UE_ARRAY_COUNT(Assets) || Index == CurrentWeaponIndex) return;
    if (CurrentWeaponIndex != INDEX_NONE && SlotMagazineAmmo.IsValidIndex(CurrentWeaponIndex))
    {
        SlotMagazineAmmo[CurrentWeaponIndex] = Weapon->GetMagazineAmmo();
        SlotReserveAmmo[CurrentWeaponIndex] = Weapon->GetReserveAmmo();
    }
    UEmberWeaponDefinition* Definition = LoadWeaponDefinition(Identifiers[Index], Assets[Index]);
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
    const bool bFirstEquip = !SlotMagazineAmmo.IsValidIndex(Index) || SlotMagazineAmmo[Index] == INDEX_NONE;
    const int32 Magazine = bFirstEquip ? Definition->MagazineCapacity : SlotMagazineAmmo[Index];
    const int32 Reserve = bFirstEquip ? 180 : SlotReserveAmmo[Index];
    if (Weapon->InitializeWeaponState(Definition, Magazine, Reserve))
    {
        CurrentWeaponIndex = Index;
        SlotMagazineAmmo[Index] = Magazine;
        SlotReserveAmmo[Index] = Reserve;
        UpdateWeaponPresentation(Index);
        UE_LOG(LogEmberCombat, Log, TEXT("Equipped weapon slot %d: magazine=%d reserve=%d"),
            Index + 1, Magazine, Reserve);
    }
}

void AEmberCharacter::UpdateWeaponPresentation(int32 Index)
{
    if (!WeaponBodyVisual || Index < 0 || Index >= 6) return;
    UStaticMesh* DesiredMesh = WeaponPresentationMeshes.IsValidIndex(Index)
        ? WeaponPresentationMeshes[Index].Get() : nullptr;
    if (!DesiredMesh)
    {
        DesiredMesh = Index == 5 && SidearmWeaponMesh
            ? SidearmWeaponMesh.Get() : PrimaryWeaponMesh.Get();
    }
    WeaponBodyVisual->SetStaticMesh(DesiredMesh);
    const FTransform PresentationTransform = WeaponPresentationTransforms.IsValidIndex(Index)
        ? WeaponPresentationTransforms[Index] : FTransform::Identity;
    ActiveWeaponPresentationTransform = PresentationTransform;
    WeaponBodyVisual->SetRelativeTransform(ActiveWeaponPresentationTransform);

    // The rifle graph is the shared locomotion/weapon-layer graph for every
    // shipped slot.  Slot-specific fire and reload clips still come from each
    // weapon definition, while retaining one cooked graph avoids a runtime
    // dependency on the template pistol graph (which is not referenced by the
    // generated map and can therefore be removed by the cooker).
    const TCHAR* AnimClassPath =
        TEXT("/Game/Variant_Shooter/Anims/ABP_TP_Rifle.ABP_TP_Rifle_C");
    if (UClass* AnimClass = LoadClass<UAnimInstance>(nullptr, AnimClassPath))
    {
        GetMesh()->SetAnimInstanceClass(AnimClass);
    }
}

void AEmberCharacter::PlayCurrentWeaponMontage(bool bReloadMontage)
{
    const UEmberWeaponDefinition* Definition = Weapon ? Weapon->GetDefinition() : nullptr;
    if (!Definition || !GetMesh()) return;
    UAnimMontage* Montage = bReloadMontage
        ? Definition->ReloadMontage.LoadSynchronous()
        : Definition->FireMontage.LoadSynchronous();
    if (Montage)
    {
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            AnimInstance->Montage_Play(Montage, 1.0f);
        }
        return;
    }
    UAnimSequence* Sequence = bReloadMontage
        ? Definition->ReloadAnimation.LoadSynchronous()
        : Definition->FireAnimation.LoadSynchronous();
    if (Sequence)
    {
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            AnimInstance->PlaySlotAnimationAsDynamicMontage(
                Sequence, TEXT("DefaultSlot"), 0.08f, 0.12f, 1.0f, 1, -1.0f, 0.0f);
        }
    }
}

void AEmberCharacter::SwapShoulder()
{
    bRightShoulder = !bRightShoulder;
}

FEmberShotRequest AEmberCharacter::BuildShotRequest() const
{
    FEmberShotRequest Request;
    Request.CameraOrigin = FollowCamera->GetComponentLocation();
    const float SpreadRadians = FMath::DegreesToRadians(Weapon ? Weapon->GetSpreadDegrees(bAiming) : 0.0f);
    Request.DesiredDirection = FMath::VRandCone(FollowCamera->GetForwardVector(), SpreadRadians);
    Request.MuzzleOrigin = WeaponBarrelVisual
        ? MuzzleFlashLight->GetComponentLocation()
        : GetActorLocation() + GetActorForwardVector() * 50.0f;
    Request.MaximumRange = Weapon ? Weapon->GetMaximumRange() : 10000.0f;
    return Request;
}

FEmberDamageResult AEmberCharacter::ReceiveEmberDamage_Implementation(const FEmberDamageSpec& DamageSpec)
{
    const FEmberDamageResult Result = DamageReceiver ? DamageReceiver->ApplyDamageSpec(DamageSpec) : FEmberDamageResult();
    if (Result.AppliedToHealth + Result.AppliedToArmor > 0.0f && GetWorld())
    {
        LastDamageTimeSeconds = GetWorld()->GetTimeSeconds();
    }
    return Result;
}
