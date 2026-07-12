#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EmberInterfaces.h"
#include "EmberCharacter.generated.h"

class UCameraComponent;
class UAnimationAsset;
class UEmberArmorComponent;
class UEmberDamageReceiverComponent;
class UEmberHealthComponent;
class UEmberInteractionComponent;
class UEmberInventoryComponent;
class UEmberWeaponComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
struct FInputActionValue;

UCLASS()
class EMBERGAMEPLAY_API AEmberCharacter : public ACharacter, public IEmberDamageable
{
    GENERATED_BODY()
public:
    AEmberCharacter();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual FEmberDamageResult ReceiveEmberDamage_Implementation(const FEmberDamageSpec& DamageSpec) override;
    UFUNCTION(BlueprintCallable) void SetAiming(bool bNewAiming);
    UFUNCTION(BlueprintCallable) void SwapShoulder();
    UFUNCTION(BlueprintPure) bool IsAiming() const { return bAiming; }
    UFUNCTION(BlueprintPure) UEmberHealthComponent* GetHealthComponent() const { return Health; }
    UFUNCTION(BlueprintPure) UEmberArmorComponent* GetArmorComponent() const { return Armor; }
    UFUNCTION(BlueprintPure) UEmberWeaponComponent* GetWeaponComponent() const { return Weapon; }
protected:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void AimStarted();
    void AimCompleted();
    void FireStarted();
    void FireCompleted();
    void Reload();
    void TogglePauseMenu();
    void SelectWeapon1();
    void SelectWeapon2();
    void SelectWeapon3();
    void SelectWeapon4();
    void SelectWeapon5();
    void SelectWeapon6();
    void EquipWeaponIndex(int32 Index);
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void StartSprint();
    void StopSprint();
    void ToggleCrouch();
    void InitializeStarterWeapon();
    FEmberShotRequest BuildShotRequest() const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UCameraComponent> FollowCamera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberHealthComponent> Health;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberArmorComponent> Armor;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberDamageReceiverComponent> DamageReceiver;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberWeaponComponent> Weapon;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberInventoryComponent> Inventory;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberInteractionComponent> Interaction;

    UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<UInputMappingContext> OnFootMapping;
    UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<UInputAction> MoveAction;
    UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<UInputAction> LookAction;
    UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<UInputAction> AimAction;
    UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<UInputAction> FireAction;
    UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<UInputAction> ReloadAction;
    UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<UInputAction> ShoulderSwapAction;

    UPROPERTY(EditDefaultsOnly, Category="Camera") float ExplorationArmLength = 350.0f;
    UPROPERTY(EditDefaultsOnly, Category="Camera") float AimArmLength = 260.0f;
    UPROPERTY(EditDefaultsOnly, Category="Camera") float ShoulderOffset = 65.0f;
    UPROPERTY(VisibleInstanceOnly, Category="Camera") bool bAiming = false;
    UPROPERTY(VisibleInstanceOnly, Category="Camera") bool bRightShoulder = true;
    UPROPERTY(EditDefaultsOnly, Category="Movement") float JogSpeed = 500.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement") float SprintSpeed = 750.0f;
    FTimerHandle AutomaticFireTimer;
    UPROPERTY(Transient) TObjectPtr<UAnimationAsset> IdleAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimationAsset> WalkAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimationAsset> JogAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimationAsset> ActiveLocomotionAnimation;
};
