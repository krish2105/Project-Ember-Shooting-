#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "EmberAssetManager.generated.h"

UCLASS()
class EMBERCORE_API UEmberAssetManager : public UAssetManager
{
    GENERATED_BODY()
public:
    static UEmberAssetManager& Get();
    virtual void StartInitialLoading() override;
};

