#include "EmberValidateContentCommandlet.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EmberEnemyArchetypeDefinition.h"
#include "EmberMissionDefinition.h"
#include "EmberPresentationDefinitions.h"
#include "EmberWeaponDefinition.h"
#include "UObject/SoftObjectPath.h"

UEmberValidateContentCommandlet::UEmberValidateContentCommandlet()
{
    IsClient = false;
    IsServer = false;
    IsEditor = true;
    LogToConsole = true;
}

int32 UEmberValidateContentCommandlet::Main(const FString& Params)
{
    FAssetRegistryModule& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    Registry.Get().SearchAllAssets(true);
    int32 Errors = 0;

    TArray<FAssetData> Weapons;
    Registry.Get().GetAssetsByClass(UEmberWeaponDefinition::StaticClass()->GetClassPathName(), Weapons, true);
    if (Weapons.Num() < 12)
    {
        UE_LOG(LogTemp, Error, TEXT("Expected 12 weapon definitions, found %d."), Weapons.Num());
        ++Errors;
    }

    TSet<FPrimaryAssetId> WeaponIds;
    for (const FAssetData& Asset : Weapons)
    {
        const UEmberWeaponDefinition* Definition = Cast<UEmberWeaponDefinition>(Asset.GetAsset());
        if (!Definition || Definition->Identifier.IsNone() || Definition->DisplayName.IsEmpty() || Definition->MagazineCapacity <= 0 || Definition->RoundsPerMinute <= 0.0f || Definition->MaximumRange <= 100.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid weapon definition: %s"), *Asset.GetObjectPathString());
            ++Errors;
        }
        else if (WeaponIds.Contains(Definition->GetPrimaryAssetId()))
        {
            UE_LOG(LogTemp, Error, TEXT("Duplicate weapon identifier: %s"), *Definition->GetPrimaryAssetId().ToString());
            ++Errors;
        }
        else WeaponIds.Add(Definition->GetPrimaryAssetId());
    }

    TArray<FAssetData> Enemies;
    Registry.Get().GetAssetsByClass(UEmberEnemyArchetypeDefinition::StaticClass()->GetClassPathName(), Enemies, true);
    if (Enemies.Num() < 6)
    {
        UE_LOG(LogTemp, Error, TEXT("Expected 6 enemy archetypes, found %d."), Enemies.Num());
        ++Errors;
    }
    TSet<FPrimaryAssetId> EnemyIds;
    for (const FAssetData& Asset : Enemies)
    {
        const UEmberEnemyArchetypeDefinition* Definition = Cast<UEmberEnemyArchetypeDefinition>(Asset.GetAsset());
        if (!Definition || Definition->Identifier.IsNone() || Definition->DisplayName.IsEmpty() || Definition->PreferredWeapon.IsNull())
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid enemy archetype: %s"), *Asset.GetObjectPathString());
            ++Errors;
        }
        else if (EnemyIds.Contains(Definition->GetPrimaryAssetId()))
        {
            UE_LOG(LogTemp, Error, TEXT("Duplicate enemy identifier: %s"), *Definition->GetPrimaryAssetId().ToString());
            ++Errors;
        }
        else EnemyIds.Add(Definition->GetPrimaryAssetId());
    }

    TArray<FAssetData> Missions;
    Registry.Get().GetAssetsByClass(UEmberMissionDefinition::StaticClass()->GetClassPathName(), Missions, true);
    if (Missions.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("No mission definition exists."));
        ++Errors;
    }
    for (const FAssetData& Asset : Missions)
    {
        const UEmberMissionDefinition* Definition = Cast<UEmberMissionDefinition>(Asset.GetAsset());
        if (!Definition || Definition->Identifier.IsNone() || Definition->Objectives.IsEmpty())
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid mission definition: %s"), *Asset.GetObjectPathString());
            ++Errors;
            continue;
        }
        TSet<FName> ObjectiveIds;
        for (const FEmberObjectiveDefinition& Objective : Definition->Objectives)
        {
            if (Objective.Identifier.IsNone() || Objective.Title.IsEmpty() || ObjectiveIds.Contains(Objective.Identifier))
            {
                UE_LOG(LogTemp, Error, TEXT("Mission %s has an invalid or duplicate objective."), *Definition->Identifier.ToString());
                ++Errors;
            }
            ObjectiveIds.Add(Objective.Identifier);
        }
        for (const FEmberObjectiveDefinition& Objective : Definition->Objectives)
        {
            for (FName Prerequisite : Objective.Prerequisites)
            {
                if (!ObjectiveIds.Contains(Prerequisite))
                {
                    UE_LOG(LogTemp, Error, TEXT("Objective %s references missing prerequisite %s."), *Objective.Identifier.ToString(), *Prerequisite.ToString());
                    ++Errors;
                }
            }
        }
    }

    TArray<FAssetData> Surfaces;
    Registry.Get().GetAssetsByClass(UEmberSurfaceResponseDefinition::StaticClass()->GetClassPathName(), Surfaces, true);
    if (Surfaces.Num() < 12)
    {
        UE_LOG(LogTemp, Error, TEXT("Expected 12 surface response definitions, found %d."), Surfaces.Num());
        ++Errors;
    }

    TArray<FAssetData> AudioEvents;
    Registry.Get().GetAssetsByClass(UEmberAudioEventDefinition::StaticClass()->GetClassPathName(), AudioEvents, true);
    if (AudioEvents.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("No audio event definitions exist."));
        ++Errors;
    }

    const FAssetData HarborMap = Registry.Get().GetAssetByObjectPath(FSoftObjectPath(TEXT("/Game/Ember/Maps/L_HarborVerticalSlice.L_HarborVerticalSlice")));
    if (!HarborMap.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Required harbor vertical-slice map is missing."));
        ++Errors;
    }

    UE_LOG(LogTemp, Display, TEXT("PROJECT EMBER content validation completed with %d error(s)."), Errors);
    return Errors == 0 ? 0 : 1;
}
