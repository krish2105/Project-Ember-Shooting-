#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
#include "GameplayTagContainer.h"
#include "UObject/PrimaryAssetId.h"
#include "UObject/SoftObjectPath.h"
#include "EmberTypes.generated.h"

UENUM(BlueprintType)
enum class EEmberFireMode : uint8
{
    Safe,
    SemiAutomatic,
    Burst,
    FullyAutomatic
};

UENUM(BlueprintType)
enum class EEmberWeaponCategory : uint8
{
    AssaultRifle,
    Carbine,
    SubmachineGun,
    Shotgun,
    MarksmanRifle,
    PrecisionRifle,
    LightMachineGun,
    Pistol,
    Throwable
};

UENUM(BlueprintType)
enum class EEmberReloadStage : uint8
{
    None,
    Started,
    MagazineRemoved,
    MagazineInserted,
    Chambered,
    Completed,
    Cancelled
};

USTRUCT(BlueprintType)
struct EMBERCORE_API FEmberShotRequest
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FVector_NetQuantize MuzzleOrigin = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite) FVector_NetQuantize CameraOrigin = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite) FVector_NetQuantizeNormal DesiredDirection = FVector::ForwardVector;
    UPROPERTY(BlueprintReadWrite) float MaximumRange = 10000.0f;
    UPROPERTY(BlueprintReadWrite) FGameplayTag WeaponTag;
    UPROPERTY(BlueprintReadWrite) FGuid InstigatorId;
};

USTRUCT(BlueprintType)
struct EMBERCORE_API FEmberShotResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) bool bFired = false;
    UPROPERTY(BlueprintReadOnly) bool bMuzzleObstructed = false;
    UPROPERTY(BlueprintReadOnly) bool bHit = false;
    UPROPERTY(BlueprintReadOnly) FVector_NetQuantize ImpactPoint = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly) FVector_NetQuantizeNormal ImpactNormal = FVector::UpVector;
    UPROPERTY(BlueprintReadOnly) FGameplayTag SurfaceTag;
    UPROPERTY(BlueprintReadOnly) TObjectPtr<AActor> HitActor = nullptr;
};

USTRUCT(BlueprintType)
struct EMBERCORE_API FEmberDamageSpec
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) float BaseDamage = 0.0f;
    UPROPERTY(BlueprintReadWrite) float ArmorModifier = 1.0f;
    UPROPERTY(BlueprintReadWrite) float BodyPartModifier = 1.0f;
    UPROPERTY(BlueprintReadWrite) float DistanceModifier = 1.0f;
    UPROPERTY(BlueprintReadWrite) float Stagger = 0.0f;
    UPROPERTY(BlueprintReadWrite) float Suppression = 0.0f;
    UPROPERTY(BlueprintReadWrite) FGameplayTag DamageType;
    UPROPERTY(BlueprintReadWrite) FGuid SourceId;
};

USTRUCT(BlueprintType)
struct EMBERCORE_API FEmberDamageResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) float AppliedToArmor = 0.0f;
    UPROPERTY(BlueprintReadOnly) float AppliedToHealth = 0.0f;
    UPROPERTY(BlueprintReadOnly) bool bKilled = false;
    UPROPERTY(BlueprintReadOnly) bool bArmoredHit = false;
};

USTRUCT(BlueprintType)
struct EMBERCORE_API FEmberSaveParticipantRecord
{
    GENERATED_BODY()

    UPROPERTY() FGuid StableId;
    UPROPERTY() FSoftObjectPath ClassPath;
    UPROPERTY() FTransform Transform;
    UPROPERTY() TArray<uint8> Payload;
};

USTRUCT(BlueprintType)
struct EMBERCORE_API FEmberCheckpointSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) int32 SchemaVersion = 1;
    UPROPERTY(BlueprintReadOnly) FGuid CheckpointId;
    UPROPERTY(BlueprintReadOnly) FTransform PlayerTransform;
    UPROPERTY(BlueprintReadOnly) float Health = 100.0f;
    UPROPERTY(BlueprintReadOnly) float Armor = 0.0f;
    UPROPERTY(BlueprintReadOnly) TArray<FPrimaryAssetId> WeaponIds;
    UPROPERTY(BlueprintReadOnly) TMap<FName, int32> AmmunitionByType;
    UPROPERTY(BlueprintReadOnly) TMap<FName, uint8> ObjectiveStates;
    UPROPERTY(BlueprintReadOnly) TArray<FEmberSaveParticipantRecord> Participants;
};
