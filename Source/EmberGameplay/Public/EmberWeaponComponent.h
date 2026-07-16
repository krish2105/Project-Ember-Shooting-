#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EmberTypes.h"
#include "EmberWeaponComponent.generated.h"

class UEmberWeaponDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEmberShotResolved, const FEmberShotResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEmberAmmoChanged, int32, Magazine, int32, Reserve);

UCLASS(ClassGroup=(Ember), meta=(BlueprintSpawnableComponent))
class EMBERGAMEPLAY_API UEmberWeaponComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UEmberWeaponComponent();
    UFUNCTION(BlueprintCallable) bool InitializeWeapon(UEmberWeaponDefinition* InDefinition, int32 InReserveAmmo);
    UFUNCTION(BlueprintCallable) bool InitializeWeaponState(UEmberWeaponDefinition* InDefinition, int32 InMagazineAmmo, int32 InReserveAmmo);
    UFUNCTION(BlueprintCallable) bool RequestFire(const FEmberShotRequest& Request);
    UFUNCTION(BlueprintCallable) void StopFire();
    UFUNCTION(BlueprintCallable) bool BeginReload();
    UFUNCTION(BlueprintCallable) void AdvanceReloadStage(EEmberReloadStage NewStage);
    UFUNCTION(BlueprintCallable) bool CancelReload();
    UFUNCTION(BlueprintPure) bool CanFire() const;
    UFUNCTION(BlueprintPure) int32 GetMagazineAmmo() const { return MagazineAmmo; }
    UFUNCTION(BlueprintPure) int32 GetReserveAmmo() const { return ReserveAmmo; }
    UFUNCTION(BlueprintPure) bool IsReloading() const { return ReloadStage != EEmberReloadStage::None; }
    UFUNCTION(BlueprintPure) EEmberReloadStage GetReloadStage() const { return ReloadStage; }
    UFUNCTION(BlueprintPure) float GetReloadProgress() const;
    UFUNCTION(BlueprintPure) UEmberWeaponDefinition* GetDefinition() const { return Definition; }
    UFUNCTION(BlueprintPure) float GetMaximumRange() const;
    UFUNCTION(BlueprintPure) FText GetWeaponDisplayName() const;
    UFUNCTION(BlueprintPure) bool IsAutomatic() const;
    UFUNCTION(BlueprintPure) float GetSpreadDegrees(bool bAiming) const;
    UFUNCTION(BlueprintPure) float GetVerticalRecoil() const;
    UFUNCTION(BlueprintPure) float GetHorizontalRecoil() const;
    UFUNCTION(BlueprintPure) bool IsMuzzleObstructed() const { return bLastMuzzleObstructed; }
    UPROPERTY(BlueprintAssignable) FEmberShotResolved OnShotResolved;
    UPROPERTY(BlueprintAssignable) FEmberAmmoChanged OnAmmoChanged;
protected:
    FEmberShotResult ResolveHitscan(const FEmberShotRequest& Request) const;
    void CompleteReload();
    UPROPERTY(VisibleInstanceOnly, Category="Weapon") TObjectPtr<UEmberWeaponDefinition> Definition;
    UPROPERTY(VisibleInstanceOnly, Category="Weapon") int32 MagazineAmmo = 0;
    UPROPERTY(VisibleInstanceOnly, Category="Weapon") int32 ReserveAmmo = 0;
    UPROPERTY(VisibleInstanceOnly, Category="Weapon") EEmberFireMode ActiveFireMode = EEmberFireMode::Safe;
    UPROPERTY(VisibleInstanceOnly, Category="Weapon") EEmberReloadStage ReloadStage = EEmberReloadStage::None;
    UPROPERTY(VisibleInstanceOnly, Category="Weapon") bool bLastMuzzleObstructed = false;
    double LastFireTimeSeconds = -TNumericLimits<double>::Max();
    double ReloadStartedAtSeconds = 0.0;
    float ReloadDurationSeconds = 0.0f;
    FTimerHandle ReloadTimer;
};
