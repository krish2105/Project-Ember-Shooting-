#include "EmberAssetManager.h"
#include "EmberLog.h"
#include "Engine/Engine.h"

UEmberAssetManager& UEmberAssetManager::Get()
{
    check(GEngine);
    UEmberAssetManager* Manager = Cast<UEmberAssetManager>(GEngine->AssetManager);
    checkf(Manager, TEXT("AssetManagerClassName must reference UEmberAssetManager."));
    return *Manager;
}

void UEmberAssetManager::StartInitialLoading()
{
    Super::StartInitialLoading();
    UE_LOG(LogEmber, Log, TEXT("PROJECT EMBER Asset Manager initialized."));
}

