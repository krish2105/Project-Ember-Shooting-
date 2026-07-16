#include "EmberBootstrapDataCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Animation/AnimSequence.h"
#include "EmberEnemyArchetypeDefinition.h"
#include "EmberMissionDefinition.h"
#include "EmberPresentationDefinitions.h"
#include "EmberWeaponDefinition.h"
#include "GameplayTagsManager.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

namespace EmberBootstrap
{
    template <typename T>
    T* CreateOrLoad(const FString& PackageName, const FString& AssetName)
    {
        const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName);
        if (T* Existing = LoadObject<T>(nullptr, *ObjectPath)) return Existing;
        UPackage* Package = CreatePackage(*PackageName);
        T* Asset = NewObject<T>(Package, *AssetName, RF_Public | RF_Standalone | RF_Transactional);
        FAssetRegistryModule::AssetCreated(Asset);
        return Asset;
    }

    bool Save(UObject* Asset)
    {
        if (!Asset) return false;
        UPackage* Package = Asset->GetPackage();
        Package->GetMetaData();
        Package->MarkPackageDirty();
        const FString FileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(FileName), true);
        FSavePackageArgs Args;
        Args.TopLevelFlags = RF_Public | RF_Standalone;
        Args.SaveFlags = SAVE_NoError;
        Args.Error = GLog;
        return UPackage::SavePackage(Package, Asset, *FileName, Args);
    }

    FGameplayTag Tag(const TCHAR* Name)
    {
        return UGameplayTagsManager::Get().RequestGameplayTag(FName(Name), true);
    }
}

UEmberBootstrapDataCommandlet::UEmberBootstrapDataCommandlet()
{
    IsClient = false;
    IsServer = false;
    IsEditor = true;
    LogToConsole = true;
}

int32 UEmberBootstrapDataCommandlet::Main(const FString& Params)
{
    using namespace EmberBootstrap;
    int32 Errors = 0;

    struct FWeaponRow
    {
        const TCHAR* Asset;
        const TCHAR* Id;
        const TCHAR* Name;
        EEmberWeaponCategory Category;
        int32 Magazine;
        float RPM;
        float Damage;
        float Range;
        TArray<EEmberFireMode> Modes;
    };

    const TArray<FWeaponRow> Weapons = {
        { TEXT("DA_Weapon_AshlineA4"), TEXT("Weapon.AshlineA4"), TEXT("Ashline A4"), EEmberWeaponCategory::AssaultRifle, 30, 720, 30, 30000, {EEmberFireMode::Safe, EEmberFireMode::SemiAutomatic, EEmberFireMode::FullyAutomatic} },
        { TEXT("DA_Weapon_BastionH7"), TEXT("Weapon.BastionH7"), TEXT("Bastion H7"), EEmberWeaponCategory::AssaultRifle, 25, 560, 39, 34000, {EEmberFireMode::Safe, EEmberFireMode::SemiAutomatic, EEmberFireMode::FullyAutomatic} },
        { TEXT("DA_Weapon_SparrowC9"), TEXT("Weapon.SparrowC9"), TEXT("Sparrow C9"), EEmberWeaponCategory::Carbine, 30, 780, 26, 24000, {EEmberFireMode::Safe, EEmberFireMode::SemiAutomatic, EEmberFireMode::FullyAutomatic} },
        { TEXT("DA_Weapon_DriftSM6"), TEXT("Weapon.DriftSM6"), TEXT("Drift SM-6"), EEmberWeaponCategory::SubmachineGun, 32, 900, 21, 15000, {EEmberFireMode::Safe, EEmberFireMode::SemiAutomatic, EEmberFireMode::FullyAutomatic} },
        { TEXT("DA_Weapon_BreachP12"), TEXT("Weapon.BreachP12"), TEXT("Breach P12"), EEmberWeaponCategory::Shotgun, 8, 75, 95, 9000, {EEmberFireMode::Safe, EEmberFireMode::SemiAutomatic} },
        { TEXT("DA_Weapon_RelayS8"), TEXT("Weapon.RelayS8"), TEXT("Relay S8"), EEmberWeaponCategory::Shotgun, 7, 240, 76, 10000, {EEmberFireMode::Safe, EEmberFireMode::SemiAutomatic} },
        { TEXT("DA_Weapon_VigilD3"), TEXT("Weapon.VigilD3"), TEXT("Vigil D3"), EEmberWeaponCategory::MarksmanRifle, 16, 300, 57, 48000, {EEmberFireMode::Safe, EEmberFireMode::SemiAutomatic} },
        { TEXT("DA_Weapon_NeedleP1"), TEXT("Weapon.NeedleP1"), TEXT("Needle P1"), EEmberWeaponCategory::PrecisionRifle, 5, 45, 105, 70000, {EEmberFireMode::Safe, EEmberFireMode::SemiAutomatic} },
        { TEXT("DA_Weapon_ForgeL5"), TEXT("Weapon.ForgeL5"), TEXT("Forge L5"), EEmberWeaponCategory::LightMachineGun, 80, 650, 33, 36000, {EEmberFireMode::Safe, EEmberFireMode::FullyAutomatic} },
        { TEXT("DA_Weapon_HarborS9"), TEXT("Weapon.HarborS9"), TEXT("Harbor S9"), EEmberWeaponCategory::Pistol, 15, 360, 25, 12000, {EEmberFireMode::Safe, EEmberFireMode::SemiAutomatic} },
        { TEXT("DA_Weapon_MonolithH2"), TEXT("Weapon.MonolithH2"), TEXT("Monolith H2"), EEmberWeaponCategory::Pistol, 9, 260, 44, 14000, {EEmberFireMode::Safe, EEmberFireMode::SemiAutomatic} },
        { TEXT("DA_Weapon_EmberFrag"), TEXT("Weapon.EmberFrag"), TEXT("Ember Fragmentation Device"), EEmberWeaponCategory::Throwable, 1, 60, 120, 2500, {EEmberFireMode::Safe, EEmberFireMode::SemiAutomatic} }
    };

    for (const FWeaponRow& Row : Weapons)
    {
        const FString Package = FString::Printf(TEXT("/Game/Ember/Weapons/%s"), Row.Asset);
        UEmberWeaponDefinition* Asset = CreateOrLoad<UEmberWeaponDefinition>(Package, Row.Asset);
        Asset->Identifier = FName(Row.Id);
        Asset->DisplayName = FText::FromString(Row.Name);
        Asset->Category = Row.Category;
        Asset->MagazineCapacity = Row.Magazine;
        Asset->RoundsPerMinute = Row.RPM;
        Asset->BaseDamage = Row.Damage;
        Asset->MaximumRange = Row.Range;
        Asset->SupportedFireModes = Row.Modes;
        Asset->AimSpreadDegrees = Row.Category == EEmberWeaponCategory::PrecisionRifle ? 0.05f : 0.25f;
        const bool bPistol = Row.Category == EEmberWeaponCategory::Pistol;
        const FString AnimationRoot = bPistol
            ? TEXT("/Game/Characters/Mannequins/Anims/Pistol")
            : TEXT("/Game/Characters/Mannequins/Anims/Rifle");
        const FString FireName = bPistol ? TEXT("MM_Pistol_Fire") : TEXT("MM_Rifle_Fire");
        const FString ReloadName = bPistol ? TEXT("MM_Pistol_Reload") : TEXT("MM_Rifle_Reload");
        Asset->FireAnimation = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
            FString::Printf(TEXT("%s/%s.%s"), *AnimationRoot, *FireName, *FireName)));
        Asset->ReloadAnimation = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
            FString::Printf(TEXT("%s/%s.%s"), *AnimationRoot, *ReloadName, *ReloadName)));
        if (!Save(Asset)) ++Errors;
    }

    struct FEnemyRow { const TCHAR* Asset; const TCHAR* Id; const TCHAR* Name; EEmberSquadRole Role; const TCHAR* WeaponAsset; float Range; };
    const TArray<FEnemyRow> Enemies = {
        {TEXT("DA_Enemy_RiflePatrol"), TEXT("Enemy.RiflePatrol"), TEXT("Rifle Patrol"), EEmberSquadRole::Rifle, TEXT("DA_Weapon_AshlineA4"), 1800},
        {TEXT("DA_Enemy_Assault"), TEXT("Enemy.Assault"), TEXT("Aggressive Assault"), EEmberSquadRole::Assault, TEXT("DA_Weapon_SparrowC9"), 1100},
        {TEXT("DA_Enemy_Suppression"), TEXT("Enemy.Suppression"), TEXT("Suppression Unit"), EEmberSquadRole::Suppression, TEXT("DA_Weapon_ForgeL5"), 2200},
        {TEXT("DA_Enemy_Marksman"), TEXT("Enemy.Marksman"), TEXT("Marksman"), EEmberSquadRole::Marksman, TEXT("DA_Weapon_VigilD3"), 4000},
        {TEXT("DA_Enemy_CloseQuarters"), TEXT("Enemy.CloseQuarters"), TEXT("Close Quarters Unit"), EEmberSquadRole::CloseQuarters, TEXT("DA_Weapon_BreachP12"), 700},
        {TEXT("DA_Enemy_SquadLeader"), TEXT("Enemy.SquadLeader"), TEXT("Squad Leader"), EEmberSquadRole::Leader, TEXT("DA_Weapon_BastionH7"), 2000}
    };
    for (const FEnemyRow& Row : Enemies)
    {
        const FString Package = FString::Printf(TEXT("/Game/Ember/AI/%s"), Row.Asset);
        UEmberEnemyArchetypeDefinition* Asset = CreateOrLoad<UEmberEnemyArchetypeDefinition>(Package, Row.Asset);
        Asset->Identifier = FName(Row.Id);
        Asset->DisplayName = FText::FromString(Row.Name);
        Asset->SquadRole = Row.Role;
        Asset->PreferredRange = Row.Range;
        Asset->PreferredWeapon = TSoftObjectPtr<UEmberWeaponDefinition>(FSoftObjectPath(FString::Printf(TEXT("/Game/Ember/Weapons/%s.%s"), Row.WeaponAsset, Row.WeaponAsset)));
        if (!Save(Asset)) ++Errors;
    }

    const TArray<const TCHAR*> SurfaceNames = {
        TEXT("Concrete"), TEXT("Metal"), TEXT("Wood"), TEXT("Glass"), TEXT("Soil"), TEXT("Mud"),
        TEXT("Sand"), TEXT("Water"), TEXT("Vegetation"), TEXT("Fabric"), TEXT("Character"), TEXT("Armor")
    };
    for (const TCHAR* Surface : SurfaceNames)
    {
        const FString AssetName = FString::Printf(TEXT("DA_Surface_%s"), Surface);
        UEmberSurfaceResponseDefinition* Asset = CreateOrLoad<UEmberSurfaceResponseDefinition>(FString::Printf(TEXT("/Game/Ember/Surfaces/%s"), *AssetName), AssetName);
        Asset->SurfaceTag = Tag(*FString::Printf(TEXT("Surface.%s"), Surface));
        Asset->DecalLifetimeSeconds = 20.0f;
        if (!Save(Asset)) ++Errors;
    }

    const TArray<const TCHAR*> AudioTags = { TEXT("Audio.Weapon.Fire"), TEXT("Audio.Weapon.Reload"), TEXT("Audio.Environment.Indoor"), TEXT("Audio.Environment.Outdoor") };
    for (const TCHAR* AudioTag : AudioTags)
    {
        FString SafeName(AudioTag);
        SafeName.ReplaceInline(TEXT("."), TEXT("_"));
        const FString AssetName = FString::Printf(TEXT("DA_%s"), *SafeName);
        UEmberAudioEventDefinition* Asset = CreateOrLoad<UEmberAudioEventDefinition>(FString::Printf(TEXT("/Game/Ember/Audio/%s"), *AssetName), AssetName);
        Asset->EventTag = Tag(AudioTag);
        if (!Save(Asset)) ++Errors;
    }

    UEmberMissionDefinition* Mission = CreateOrLoad<UEmberMissionDefinition>(TEXT("/Game/Ember/Missions/DA_Mission_Harbor"), TEXT("DA_Mission_Harbor"));
    Mission->Identifier = TEXT("Mission.HarborVerticalSlice");
    Mission->DisplayName = FText::FromString(TEXT("Embers at the Waterline"));
    Mission->Objectives.Reset();
    const TArray<TTuple<const TCHAR*, EEmberObjectiveType, bool>> ObjectiveRows = {
        {TEXT("Insertion"), EEmberObjectiveType::ReachLocation, false},
        {TEXT("FirstPatrol"), EEmberObjectiveType::ClearArea, false},
        {TEXT("StealthRoute"), EEmberObjectiveType::OptionalDiscovery, true},
        {TEXT("Warehouse"), EEmberObjectiveType::Interact, false},
        {TEXT("MidpointCheckpoint"), EEmberObjectiveType::ReachLocation, false},
        {TEXT("CraneOverlook"), EEmberObjectiveType::ClearArea, false},
        {TEXT("ContainerArena"), EEmberObjectiveType::ClearArea, false},
        {TEXT("SecondaryManifest"), EEmberObjectiveType::RetrieveItem, true},
        {TEXT("Extraction"), EEmberObjectiveType::Extract, false},
        {TEXT("Results"), EEmberObjectiveType::ReachLocation, false}
    };
    FName PreviousRequired = NAME_None;
    for (const auto& Row : ObjectiveRows)
    {
        FEmberObjectiveDefinition Objective;
        Objective.Identifier = FName(Row.Get<0>());
        Objective.Type = Row.Get<1>();
        Objective.Title = FText::FromString(FString(Row.Get<0>()));
        Objective.Description = FText::FromString(FString::Printf(TEXT("Complete %s."), Row.Get<0>()));
        Objective.bOptional = Row.Get<2>();
        if (!Objective.bOptional && !PreviousRequired.IsNone()) Objective.Prerequisites.Add(PreviousRequired);
        if (!Objective.bOptional) PreviousRequired = Objective.Identifier;
        Mission->Objectives.Add(Objective);
    }
    if (!Save(Mission)) ++Errors;

    UE_LOG(LogTemp, Display, TEXT("PROJECT EMBER data bootstrap completed with %d error(s)."), Errors);
    return Errors == 0 ? 0 : 1;
}
