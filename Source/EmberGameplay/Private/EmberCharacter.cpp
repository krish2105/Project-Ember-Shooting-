#include "EmberCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EmberArmorComponent.h"
#include "EmberDamageReceiverComponent.h"
#include "EmberHealthComponent.h"
#include "EmberInteractionComponent.h"
#include "EmberInventoryComponent.h"
#include "EmberWeaponComponent.h"
#include "EmberWeaponDefinition.h"
#include "Engine/AssetManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AEmberCharacter::AEmberCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
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
}

void AEmberCharacter::BeginPlay()
{
    Super::BeginPlay();
    GetCharacterMovement()->MaxWalkSpeed = JogSpeed;
    InitializeStarterWeapon();
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (OnFootMapping) Subsystem->AddMappingContext(OnFootMapping, 0);
        }
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
void AEmberCharacter::StartSprint() { GetCharacterMovement()->MaxWalkSpeed = SprintSpeed; }
void AEmberCharacter::StopSprint() { GetCharacterMovement()->MaxWalkSpeed = JogSpeed; }
void AEmberCharacter::ToggleCrouch() { bIsCrouched ? UnCrouch() : Crouch(); }

void AEmberCharacter::InitializeStarterWeapon()
{
    if (!Weapon) return;
    const FPrimaryAssetId StarterId(TEXT("EmberWeapon"), TEXT("Weapon.AshlineA4"));
    const FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(StarterId);
    UEmberWeaponDefinition* Starter = Cast<UEmberWeaponDefinition>(AssetPath.TryLoad());
    if (!Starter)
    {
        Starter = NewObject<UEmberWeaponDefinition>(this, TEXT("FallbackStarterWeapon"));
        Starter->Identifier = TEXT("Weapon.FallbackStarter");
        Starter->DisplayName = FText::FromString(TEXT("Fallback Carbine"));
        Starter->SupportedFireModes = { EEmberFireMode::SemiAutomatic };
    }
    Weapon->InitializeWeapon(Starter, 180);
}

void AEmberCharacter::SetAiming(bool bNewAiming)
{
    bAiming = bNewAiming;
    CameraBoom->TargetArmLength = bAiming ? AimArmLength : ExplorationArmLength;
}

void AEmberCharacter::AimStarted() { SetAiming(true); }
void AEmberCharacter::AimCompleted() { SetAiming(false); }
void AEmberCharacter::FireStarted()
{
    if (!Weapon) return;
    Weapon->RequestFire(BuildShotRequest());
    if (Weapon->IsAutomatic())
        GetWorldTimerManager().SetTimer(AutomaticFireTimer, [this]() { if (Weapon) Weapon->RequestFire(BuildShotRequest()); }, 0.06f, true);
}
void AEmberCharacter::FireCompleted()
{
    GetWorldTimerManager().ClearTimer(AutomaticFireTimer);
    if (Weapon) Weapon->StopFire();
}
void AEmberCharacter::Reload() { if (Weapon) Weapon->BeginReload(); }

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
    if (UEmberWeaponDefinition* Definition = LoadObject<UEmberWeaponDefinition>(nullptr, *Path))
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
    Request.MuzzleOrigin = GetMesh() ? GetMesh()->GetSocketLocation(TEXT("Muzzle")) : GetActorLocation() + GetActorForwardVector() * 50.0f;
    Request.MaximumRange = Weapon ? Weapon->GetMaximumRange() : 10000.0f;
    return Request;
}

FEmberDamageResult AEmberCharacter::ReceiveEmberDamage_Implementation(const FEmberDamageSpec& DamageSpec)
{
    return DamageReceiver ? DamageReceiver->ApplyDamageSpec(DamageSpec) : FEmberDamageResult();
}
