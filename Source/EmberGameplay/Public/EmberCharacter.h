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
class UStaticMesh;
class UPointLightComponent;

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
    UFUNCTION(BlueprintCallable) void SetToggleAimEnabled(bool bEnabled) { bToggleAimInput = bEnabled; }
    UFUNCTION(BlueprintPure) bool IsToggleAimEnabled() const { return bToggleAimInput; }
    UFUNCTION(BlueprintPure) bool IsFiringInputHeld() const { return bFireInputHeld; }
    UFUNCTION(BlueprintPure) bool HasHostileUnderCrosshair() const;
    UFUNCTION(BlueprintPure) bool ShouldShowHitMarker() const;
    UFUNCTION(BlueprintPure) float GetDamageFeedbackAlpha() const;
    UFUNCTION(BlueprintPure) bool IsTacticalOverlayVisible() const { return bTacticalOverlayVisible; }
    UFUNCTION(BlueprintPure) bool IsControlsOverlayVisible() const { return bControlsOverlayVisible; }
    UFUNCTION(BlueprintPure) float GetRecoilFeedback() const { return WeaponVisualRecoil; }
    UFUNCTION(BlueprintPure) UEmberHealthComponent* GetHealthComponent() const { return Health; }
    UFUNCTION(BlueprintPure) UEmberArmorComponent* GetArmorComponent() const { return Armor; }
    UFUNCTION(BlueprintPure) UEmberWeaponComponent* GetWeaponComponent() const { return Weapon; }
    UFUNCTION(BlueprintPure) int32 GetCurrentWeaponIndex() const { return CurrentWeaponIndex; }
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
    void Interact();
    void MeleeAttack();
    void TogglePauseMenu();
    void ToggleTacticalOverlay();
    void ToggleControlsOverlay();
    void CycleWeaponNext();
    void CycleWeaponPrevious();
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
    void PlayCurrentWeaponMontage(bool bReloadMontage);
    void InitializeGunshotAudio();
    void ResetMuzzleFlash();
    void ResetShotTracer();
    void ResetImpactFeedback();
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
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon|Feedback") TObjectPtr<UStaticMeshComponent> ShotTracerVisual;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon") TObjectPtr<UPointLightComponent> MuzzleFlashLight;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon|Feedback") TObjectPtr<UPointLightComponent> ImpactFeedbackLight;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon|Feedback") TObjectPtr<class UAudioComponent> GunshotAudio;
    UPROPERTY(Transient) TObjectPtr<class USoundWaveProcedural> GunshotWave;
    /** One independently authored presentation entry per mission weapon slot. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Presentation") TArray<TObjectPtr<UStaticMesh>> WeaponPresentationMeshes;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Presentation") TArray<FTransform> WeaponPresentationTransforms;
    /** Compatibility fallback while final per-slot assets are being approved. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Presentation") TObjectPtr<UStaticMesh> PrimaryWeaponMesh;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Presentation") TObjectPtr<UStaticMesh> SidearmWeaponMesh;

    UPROPERTY(EditDefaultsOnly, Category="Camera") float ExplorationArmLength = 310.0f;
    UPROPERTY(EditDefaultsOnly, Category="Camera") float AimArmLength = 215.0f;
    UPROPERTY(EditDefaultsOnly, Category="Camera") float ShoulderOffset = 78.0f;
    UPROPERTY(EditDefaultsOnly, Category="Camera", meta=(ClampMin="1.0")) float CameraBlendSpeed = 12.0f;
    /** Accessibility option. The gameplay default remains hold-to-aim. */
    UPROPERTY(EditDefaultsOnly, Category="Accessibility") bool bToggleAimInput = false;
    UPROPERTY(VisibleInstanceOnly, Category="Camera") bool bAiming = false;
    UPROPERTY(VisibleInstanceOnly, Category="Camera") bool bRightShoulder = true;
    UPROPERTY(EditDefaultsOnly, Category="Movement") float JogSpeed = 500.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement") float SprintSpeed = 750.0f;
    FTimerHandle AutomaticFireTimer;
    FTimerHandle MuzzleFlashTimer;
    FTimerHandle ShotTracerTimer;
    FTimerHandle ImpactFeedbackTimer;
    FTimerHandle MeleeCooldownTimer;
    bool bFireInputHeld = false;
    bool bMeleeReady = true;
    bool bTacticalOverlayVisible = false;
    bool bControlsOverlayVisible = false;
    double LastHitTimeSeconds = -TNumericLimits<double>::Max();
    double LastDamageTimeSeconds = -TNumericLimits<double>::Max();
    FEmberShotRequest LastShotRequest;
    TArray<uint8> GunshotPCM;
    TArray<int32> SlotMagazineAmmo;
    TArray<int32> SlotReserveAmmo;
    int32 CurrentWeaponIndex = INDEX_NONE;
    FTransform ActiveWeaponPresentationTransform = FTransform::Identity;
    float WeaponVisualRecoil = 0.0f;
};
