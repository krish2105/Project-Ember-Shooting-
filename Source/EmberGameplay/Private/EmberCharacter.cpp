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
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->MaxWalkSpeed = JogSpeed;
    Movement->GravityScale = 1.0f;
    Movement->SetPlaneConstraintEnabled(false);
    Movement->SetMovementMode(MOVE_Walking);
    InitializeStarterWeapon();
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
    if (!Weapon) return;
    UEmberWeaponDefinition* Starter = LoadWeaponDefinition(
        TEXT("Weapon.AshlineA4"), TEXT("DA_Weapon_AshlineA4"));
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
    bFireInputHeld = true;
    if (Weapon->RequestFire(BuildShotRequest())) PlayGunshotFeedback();
    if (Weapon->IsAutomatic())
        GetWorldTimerManager().SetTimer(AutomaticFireTimer, [this]() {
            if (Weapon && Weapon->RequestFire(BuildShotRequest())) PlayGunshotFeedback();
        }, 0.06f, true);
}
void AEmberCharacter::FireCompleted()
{
    bFireInputHeld = false;
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
    static const TCHAR* Identifiers[] = {
        TEXT("Weapon.AshlineA4"), TEXT("Weapon.SparrowC9"), TEXT("Weapon.BreachP12"),
        TEXT("Weapon.VigilD3"), TEXT("Weapon.ForgeL5"), TEXT("Weapon.HarborS9")
    };
    if (!Weapon || Index < 0 || Index >= UE_ARRAY_COUNT(Assets)) return;
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
