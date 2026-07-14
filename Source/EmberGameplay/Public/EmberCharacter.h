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
class USpringArmComponent;
class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class EMBERGAMEPLAY_API AEmberCharacter : public ACharacter, public IEmberDamageable
{
    GENERATED_BODY()
public:
    AEmberCharacter();
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual FEmberDamageResult ReceiveEmberDamage_Implementation(const FEmberDamageSpec& DamageSpec) override;
    UFUNCTION(BlueprintCallable) void SetAiming(bool bNewAiming);
    UFUNCTION(BlueprintCallable) void SwapShoulder();
    UFUNCTION(BlueprintPure) bool IsAiming() const { return bAiming; }
    UFUNCTION(BlueprintPure) bool IsFiringInputHeld() const { return bFireInputHeld; }
    UFUNCTION(BlueprintPure) bool HasHostileUnderCrosshair() const;
    UFUNCTION(BlueprintPure) bool ShouldShowHitMarker() const;
    UFUNCTION(BlueprintPure) UEmberHealthComponent* GetHealthComponent() const { return Health; }
    UFUNCTION(BlueprintPure) UEmberArmorComponent* GetArmorComponent() const { return Armor; }
    UFUNCTION(BlueprintPure) UEmberWeaponComponent* GetWeaponComponent() const { return Weapon; }
    void WriteWeaponCheckpoint(FEmberCheckpointSnapshot& Snapshot) const;
    bool RestoreWeaponCheckpoint(const FEmberCheckpointSnapshot& Snapshot);
protected:
    void AimStarted();
    void AimCompleted();
    void FireStarted();
    void FireCompleted();
    bool TryFire();
    UFUNCTION() void HandleShotResolved(const FEmberShotResult& Result);
    void Reload();
    void TogglePauseMenu();
    void SelectWeapon1();
    void SelectWeapon2();
    void SelectWeapon3();
    void SelectWeapon4();
    void SelectWeapon5();
    void SelectWeapon6();
    void EquipWeaponIndex(int32 Index);
    void UpdateWeaponPresentation(int32 Index);
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void StartSprint();
    void StopSprint();
    void ToggleCrouch();
    void InitializeStarterWeapon();
    void PlayGunshotFeedback();
    void ResetMuzzleFlash();
    FEmberShotRequest BuildShotRequest() const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UCameraComponent> FollowCamera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberHealthComponent> Health;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberArmorComponent> Armor;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberDamageReceiverComponent> DamageReceiver;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberWeaponComponent> Weapon;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberInventoryComponent> Inventory;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberInteractionComponent> Interaction;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon") TObjectPtr<UStaticMeshComponent> WeaponBodyVisual;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon") TObjectPtr<UStaticMeshComponent> WeaponBarrelVisual;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon") TObjectPtr<UStaticMeshComponent> WeaponStockVisual;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon") TObjectPtr<UStaticMeshComponent> WeaponMagazineVisual;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon") TObjectPtr<UStaticMeshComponent> WeaponSightVisual;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon") TObjectPtr<UPointLightComponent> MuzzleFlashLight;

    UPROPERTY(EditDefaultsOnly, Category="Camera") float ExplorationArmLength = 350.0f;
    UPROPERTY(EditDefaultsOnly, Category="Camera") float AimArmLength = 260.0f;
    UPROPERTY(EditDefaultsOnly, Category="Camera") float ShoulderOffset = 65.0f;
    UPROPERTY(VisibleInstanceOnly, Category="Camera") bool bAiming = false;
    UPROPERTY(VisibleInstanceOnly, Category="Camera") bool bRightShoulder = true;
    UPROPERTY(EditDefaultsOnly, Category="Movement") float JogSpeed = 500.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement") float SprintSpeed = 750.0f;
    FTimerHandle AutomaticFireTimer;
    FTimerHandle MuzzleFlashTimer;
    bool bFireInputHeld = false;
    double LastHitTimeSeconds = -TNumericLimits<double>::Max();
    FEmberShotRequest LastShotRequest;
    TArray<int32> SlotMagazineAmmo;
    TArray<int32> SlotReserveAmmo;
    int32 CurrentWeaponIndex = INDEX_NONE;
};
