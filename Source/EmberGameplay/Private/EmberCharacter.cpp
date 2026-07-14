#include "EmberCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
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
    PrimaryActorTick.bCanEverTick = false;
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
    // The presentation mesh follows Manny's right hand. Its actual licensed
    // mesh is assigned by the generated character Blueprint, keeping content
    // references out of runtime gameplay code.
    WeaponBodyVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponBodyVisual"));
    WeaponBodyVisual->SetupAttachment(GetMesh(), TEXT("hand_r"));
    WeaponBodyVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponBodyVisual->SetRelativeLocation(FVector::ZeroVector);
    WeaponBodyVisual->SetRelativeRotation(FRotator::ZeroRotator);
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
    MuzzleFlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("MuzzleFlashLight"));
    MuzzleFlashLight->SetupAttachment(WeaponBodyVisual);
    MuzzleFlashLight->SetRelativeLocation(FVector(105.0f, 0.0f, 10.0f));
    MuzzleFlashLight->SetAttenuationRadius(450.0f);
    MuzzleFlashLight->SetLightColor(FLinearColor(1.0f, 0.35f, 0.04f));
    MuzzleFlashLight->SetIntensity(0.0f);
}

void AEmberCharacter::BeginPlay()
{
    Super::BeginPlay();
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->MaxWalkSpeed = JogSpeed;
    Movement->GravityScale = 1.0f;
    Movement->SetPlaneConstraintEnabled(false);
    Movement->SetMovementMode(MOVE_Walking);
    InitializeStarterWeapon();
    Weapon->OnShotResolved.AddDynamic(this, &AEmberCharacter::HandleShotResolved);
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
        PC->SetIgnoreLookInput(false);
        PC->SetIgnoreMoveInput(false);
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
    CameraBoom->TargetArmLength = bAiming ? AimArmLength : ExplorationArmLength;
    CameraBoom->SocketOffset.Z = bAiming ? 58.0f : 70.0f;
    FollowCamera->SetFieldOfView(bAiming ? 70.0f : 86.0f);
    bUseControllerRotationYaw = bAiming;
    GetCharacterMovement()->bOrientRotationToMovement = !bAiming;
}

// Toggle aiming is intentional: a normal right-click visibly enters/exits ADS
// and does not depend on the first click also being used to capture the window.
void AEmberCharacter::AimStarted()
{
    SetAiming(!bAiming);
    UE_LOG(LogEmberCombat, Log, TEXT("Player aim toggled: %s"), bAiming ? TEXT("AIM") : TEXT("HIP"));
}
void AEmberCharacter::AimCompleted() {}
void AEmberCharacter::FireStarted()
{
    if (!Weapon) return;
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
    UE_LOG(LogEmberCombat, Log, TEXT("Player reload requested: %s"), bStarted ? TEXT("STARTED") : TEXT("REJECTED"));
}

bool AEmberCharacter::TryFire()
{
    if (!Weapon) return false;
    LastShotRequest = BuildShotRequest();
    if (!Weapon->RequestFire(LastShotRequest)) return false;
    UE_LOG(LogEmberCombat, Log, TEXT("Player weapon fired; magazine=%d reserve=%d"),
        Weapon->GetMagazineAmmo(), Weapon->GetReserveAmmo());
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
    if (TracerMesh && Distance > 1.0f)
    {
        UStaticMeshComponent* Tracer = NewObject<UStaticMeshComponent>(this);
        Tracer->SetStaticMesh(TracerMesh);
        Tracer->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Tracer->SetCastShadow(false);
        Tracer->RegisterComponent();
        Tracer->SetWorldLocation(Start + Delta * 0.5f);
        Tracer->SetWorldRotation(Delta.Rotation());
        Tracer->SetWorldScale3D(FVector(Distance / 100.0f, 0.012f, 0.012f));
        FTimerHandle TracerTimer;
        TWeakObjectPtr<UStaticMeshComponent> WeakTracer(Tracer);
        GetWorldTimerManager().SetTimer(TracerTimer, [WeakTracer]() {
            if (WeakTracer.IsValid()) WeakTracer->DestroyComponent();
        }, 0.055f, false);
    }
    if (Result.bHit)
    {
        LastHitTimeSeconds = GetWorld()->GetTimeSeconds();
        UPointLightComponent* Impact = NewObject<UPointLightComponent>(this);
        Impact->SetLightColor(Result.HitActor && Result.HitActor->ActorHasTag(TEXT("EmberEnemy"))
            ? FLinearColor(1.0f, 0.05f, 0.02f) : FLinearColor(1.0f, 0.55f, 0.08f));
        Impact->SetIntensity(5000.0f);
        Impact->SetAttenuationRadius(240.0f);
        Impact->RegisterComponent();
        Impact->SetWorldLocation(FVector(Result.ImpactPoint) + FVector(Result.ImpactNormal) * 3.0f);
        FTimerHandle ImpactTimer;
        TWeakObjectPtr<UPointLightComponent> WeakImpact(Impact);
        GetWorldTimerManager().SetTimer(ImpactTimer, [WeakImpact]() {
            if (WeakImpact.IsValid()) WeakImpact->DestroyComponent();
        }, 0.09f, false);
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
        PC->AddPitchInput(-(Weapon ? Weapon->GetVerticalRecoil() : 0.65f));
        const float Horizontal = Weapon ? Weapon->GetHorizontalRecoil() : 0.18f;
        PC->AddYawInput(FMath::FRandRange(-Horizontal, Horizontal));
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
    WeaponBodyVisual->SetRelativeTransform(PresentationTransform);
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
    return DamageReceiver ? DamageReceiver->ApplyDamageSpec(DamageSpec) : FEmberDamageResult();
}
