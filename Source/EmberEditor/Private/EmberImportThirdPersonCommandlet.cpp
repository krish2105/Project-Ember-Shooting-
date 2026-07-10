#include "EmberImportThirdPersonCommandlet.h"
#include "FeaturePackContentSource.h"
#include "Misc/Paths.h"

UEmberImportThirdPersonCommandlet::UEmberImportThirdPersonCommandlet()
{
    IsClient = false;
    IsServer = false;
    IsEditor = true;
    LogToConsole = true;
}

int32 UEmberImportThirdPersonCommandlet::Main(const FString& Params)
{
    const FString PackPath = FPaths::Combine(FPaths::FeaturePackDir(), TEXT("TP_ThirdPerson.upack"));
    if (!FPaths::FileExists(PackPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Third Person feature pack is missing: %s"), *PackPath);
        return 1;
    }

    FFeaturePackContentSource FeaturePack(PackPath);
    if (!FeaturePack.IsDataValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Third Person feature pack is invalid."));
        return 1;
    }
    if (!FeaturePack.InstallToProject(TEXT("/Game")))
    {
        UE_LOG(LogTemp, Error, TEXT("Third Person feature pack import failed."));
        return 1;
    }

    UE_LOG(LogTemp, Display, TEXT("Third Person feature pack imported through Unreal Editor."));
    return 0;
}
