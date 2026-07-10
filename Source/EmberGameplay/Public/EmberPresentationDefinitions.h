#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "EmberPresentationDefinitions.generated.h"

class UMaterialInterface;
class UNiagaraSystem;
class USoundBase;

UCLASS(BlueprintType)
class EMBERGAMEPLAY_API UEmberSurfaceResponseDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId(TEXT("EmberSurface"), SurfaceTag.IsValid() ? SurfaceTag.GetTagName() : GetFName()); }
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FGameplayTag SurfaceTag;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TSoftObjectPtr<UNiagaraSystem> ImpactEffect;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TSoftObjectPtr<UMaterialInterface> DecalMaterial;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TSoftObjectPtr<USoundBase> ImpactSound;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.0")) float DecalLifetimeSeconds = 20.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) int32 MinimumScalabilityLevel = 0;
};

UCLASS(BlueprintType)
class EMBERGAMEPLAY_API UEmberAudioEventDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId(TEXT("EmberAudio"), EventTag.IsValid() ? EventTag.GetTagName() : GetFName()); }
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FGameplayTag EventTag;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TArray<TSoftObjectPtr<USoundBase>> Variations;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0")) float Volume = 1.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.5", ClampMax="2.0")) float MinimumPitch = 0.96f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.5", ClampMax="2.0")) float MaximumPitch = 1.04f;
};
