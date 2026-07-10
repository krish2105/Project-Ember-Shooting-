#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EmberMissionDefinition.generated.h"

UENUM(BlueprintType)
enum class EEmberObjectiveType : uint8
{
    ReachLocation, EliminateTarget, ClearArea, DefendArea, Interact,
    RetrieveItem, Sabotage, Extract, OptionalDiscovery, OptionalRescue
};

UENUM(BlueprintType)
enum class EEmberObjectiveState : uint8
{
    Inactive, Active, Completed, Failed
};

USTRUCT(BlueprintType)
struct EMBERMISSION_API FEmberObjectiveDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FName Identifier;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) EEmberObjectiveType Type = EEmberObjectiveType::ReachLocation;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FText Title;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FText Description;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) bool bOptional = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TArray<FName> Prerequisites;
};

UCLASS(BlueprintType)
class EMBERMISSION_API UEmberMissionDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId(TEXT("EmberMission"), Identifier.IsNone() ? GetFName() : Identifier); }
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FName Identifier;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TArray<FEmberObjectiveDefinition> Objectives;
};
