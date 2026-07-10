#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EmberEnemyArchetypeDefinition.generated.h"

class UEmberWeaponDefinition;

UENUM(BlueprintType)
enum class EEmberSquadRole : uint8
{
    Rifle,
    Assault,
    Suppression,
    Marksman,
    CloseQuarters,
    Leader
};

UCLASS(BlueprintType)
class EMBERAI_API UEmberEnemyArchetypeDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId(TEXT("EmberEnemy"), Identifier.IsNone() ? GetFName() : Identifier); }
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FName Identifier;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) EEmberSquadRole SquadRole = EEmberSquadRole::Rifle;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TSoftObjectPtr<UEmberWeaponDefinition> PreferredWeapon;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.0")) float ReactionDelaySeconds = 0.35f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0")) float AimError = 0.25f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0")) float SuppressionTolerance = 0.5f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0")) float FlankPreference = 0.5f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="100.0")) float PreferredRange = 1500.0f;
};
