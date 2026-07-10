#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EmberTypes.h"
#include "EmberWeaponDefinition.generated.h"

class UAnimMontage;
class UCurveFloat;
class UNiagaraSystem;
class USkeletalMesh;
class USoundBase;
class UTexture2D;

UCLASS(BlueprintType)
class EMBERGAMEPLAY_API UEmberWeaponDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(TEXT("EmberWeapon"), Identifier.IsNone() ? GetFName() : Identifier);
    }

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity") FName Identifier;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity") FText DisplayName;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity") EEmberWeaponCategory Category = EEmberWeaponCategory::AssaultRifle;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity") FGameplayTag WeaponTag;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation") TSoftObjectPtr<USkeletalMesh> WeaponMesh;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation") TSoftObjectPtr<UTexture2D> UIIcon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fire") TArray<EEmberFireMode> SupportedFireModes = { EEmberFireMode::SemiAutomatic };
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fire", meta=(ClampMin="1.0")) float RoundsPerMinute = 600.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fire", meta=(ClampMin="1")) int32 MagazineCapacity = 30;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fire") bool bSupportsChamberedRound = true;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fire") bool bUsesProjectile = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fire", meta=(ClampMin="1.0")) float MuzzleVelocity = 90000.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fire", meta=(ClampMin="100.0")) float MaximumRange = 30000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0.0")) float BaseDamage = 30.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage") TSoftObjectPtr<UCurveFloat> DistanceFalloff;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0.0")) float ArmorModifier = 0.8f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0.0")) float HeadModifier = 2.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0.0")) float LimbModifier = 0.75f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0.0")) float Stagger = 10.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0.0")) float Suppression = 10.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Handling", meta=(ClampMin="0.0")) float HipSpreadDegrees = 2.5f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Handling", meta=(ClampMin="0.0")) float AimSpreadDegrees = 0.25f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Handling", meta=(ClampMin="0.0")) float VerticalRecoil = 1.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Handling", meta=(ClampMin="0.0")) float HorizontalRecoil = 0.35f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Handling", meta=(ClampMin="0.0")) float RecoilRecoverySeconds = 0.18f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Handling", meta=(ClampMin="40.0", ClampMax="100.0")) float AimFieldOfView = 70.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Timing", meta=(ClampMin="0.0")) float EmptyReloadSeconds = 2.4f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Timing", meta=(ClampMin="0.0")) float TacticalReloadSeconds = 2.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Timing", meta=(ClampMin="0.0")) float EquipSeconds = 0.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation") TSoftObjectPtr<UAnimMontage> FireMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation") TSoftObjectPtr<UAnimMontage> ReloadMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation") TSoftObjectPtr<UNiagaraSystem> MuzzleEffect;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation") TSoftObjectPtr<USoundBase> FireSound;
};
