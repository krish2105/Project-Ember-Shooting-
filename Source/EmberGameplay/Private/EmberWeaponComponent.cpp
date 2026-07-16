#include "EmberWeaponComponent.h"
#include "EmberBallisticsLibrary.h"
#include "EmberInterfaces.h"
#include "EmberWeaponDefinition.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

UEmberWeaponComponent::UEmberWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UEmberWeaponComponent::InitializeWeapon(UEmberWeaponDefinition* InDefinition, int32 InReserveAmmo)
{
    return InitializeWeaponState(InDefinition,
        IsValid(InDefinition) ? InDefinition->MagazineCapacity : 0, InReserveAmmo);
}

bool UEmberWeaponComponent::InitializeWeaponState(
    UEmberWeaponDefinition* InDefinition, int32 InMagazineAmmo, int32 InReserveAmmo)
{
    if (!IsValid(InDefinition) || InDefinition->MagazineCapacity <= 0 || InDefinition->RoundsPerMinute <= 0.0f) return false;
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(ReloadTimer);
    Definition = InDefinition;
    MagazineAmmo = FMath::Clamp(InMagazineAmmo, 0, Definition->MagazineCapacity);
    ReserveAmmo = FMath::Max(0, InReserveAmmo);
    ActiveFireMode = EEmberFireMode::Safe;
    for (EEmberFireMode Mode : Definition->SupportedFireModes)
    {
        if (Mode != EEmberFireMode::Safe)
        {
            ActiveFireMode = Mode;
            break;
        }
    }
    ReloadStage = EEmberReloadStage::None;
    ReloadDurationSeconds = 0.0f;
    OnAmmoChanged.Broadcast(MagazineAmmo, ReserveAmmo);
    return true;
}

bool UEmberWeaponComponent::CanFire() const
{
    if (!Definition || ActiveFireMode == EEmberFireMode::Safe || ReloadStage != EEmberReloadStage::None || MagazineAmmo <= 0) return false;
    const UWorld* World = GetWorld();
    if (!World) return false;
    return World->GetTimeSeconds() - LastFireTimeSeconds + KINDA_SMALL_NUMBER >= UEmberBallisticsLibrary::SecondsPerShot(Definition->RoundsPerMinute);
}

float UEmberWeaponComponent::GetMaximumRange() const
{
    return Definition ? Definition->MaximumRange : 10000.0f;
}

FText UEmberWeaponComponent::GetWeaponDisplayName() const
{
    return Definition ? Definition->DisplayName : FText::FromString(TEXT("Unarmed"));
}

bool UEmberWeaponComponent::IsAutomatic() const
{
    return ActiveFireMode == EEmberFireMode::FullyAutomatic || ActiveFireMode == EEmberFireMode::Burst;
}

float UEmberWeaponComponent::GetSpreadDegrees(bool bAiming) const
{
    if (!Definition) return bAiming ? 0.25f : 2.5f;
    return bAiming ? Definition->AimSpreadDegrees : Definition->HipSpreadDegrees;
}

float UEmberWeaponComponent::GetVerticalRecoil() const
{
    return Definition ? Definition->VerticalRecoil : 1.0f;
}

float UEmberWeaponComponent::GetHorizontalRecoil() const
{
    return Definition ? Definition->HorizontalRecoil : 0.35f;
}

bool UEmberWeaponComponent::RequestFire(const FEmberShotRequest& Request)
{
    if (!CanFire()) return false;
    LastFireTimeSeconds = GetWorld()->GetTimeSeconds();
    --MagazineAmmo;
    FEmberShotResult Result = ResolveHitscan(Request);
    Result.bFired = true;
    bLastMuzzleObstructed = Result.bMuzzleObstructed;
    OnAmmoChanged.Broadcast(MagazineAmmo, ReserveAmmo);
    OnShotResolved.Broadcast(Result);
    return true;
}

void UEmberWeaponComponent::StopFire() {}

FEmberShotResult UEmberWeaponComponent::ResolveHitscan(const FEmberShotRequest& Request) const
{
    FEmberShotResult Result;
    UWorld* World = GetWorld();
    if (!World || !Definition) return Result;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(EmberCameraAim), true, GetOwner());
    FHitResult CameraHit;
    const float ValidatedRange = FMath::Clamp(Request.MaximumRange, 100.0f, Definition->MaximumRange);
    const FVector CameraEnd = Request.CameraOrigin + Request.DesiredDirection.GetSafeNormal() * ValidatedRange;
    World->LineTraceSingleByChannel(CameraHit, Request.CameraOrigin, CameraEnd, ECC_Visibility, Params);
    const FVector DesiredPoint = CameraHit.bBlockingHit ? CameraHit.ImpactPoint : CameraEnd;

    FHitResult MuzzleHit;
    const FVector MuzzleDirection = (DesiredPoint - Request.MuzzleOrigin).GetSafeNormal();
    const FVector MuzzleEnd = Request.MuzzleOrigin + MuzzleDirection * ValidatedRange;
    World->LineTraceSingleByChannel(MuzzleHit, Request.MuzzleOrigin, MuzzleEnd, ECC_Visibility, Params);
    if (!MuzzleHit.bBlockingHit) return Result;

    Result.bHit = true;
    Result.ImpactPoint = MuzzleHit.ImpactPoint;
    Result.ImpactNormal = MuzzleHit.ImpactNormal;
    Result.HitActor = MuzzleHit.GetActor();
    const float DesiredMuzzleDistance = FVector::Distance(Request.MuzzleOrigin, DesiredPoint);
    const float MuzzleDistance = FVector::Distance(Request.MuzzleOrigin, MuzzleHit.ImpactPoint);
    Result.bMuzzleObstructed = MuzzleDistance + 5.0f < DesiredMuzzleDistance;

    AActor* HitActor = MuzzleHit.GetActor();
    if (HitActor && HitActor->GetClass()->ImplementsInterface(UEmberDamageable::StaticClass()))
    {
        FEmberDamageSpec Damage;
        Damage.BaseDamage = Definition->BaseDamage;
        Damage.ArmorModifier = Definition->ArmorModifier;
        Damage.DistanceModifier = 1.0f;
        Damage.Stagger = Definition->Stagger;
        Damage.Suppression = Definition->Suppression;
        Damage.ShotDirection = MuzzleDirection;
        Damage.ImpactPoint = MuzzleHit.ImpactPoint;
        Damage.SourceId = Request.InstigatorId;
        const FEmberDamageResult DamageResult = IEmberDamageable::Execute_ReceiveEmberDamage(HitActor, Damage);
        Result.bDamagedActor = DamageResult.AppliedToHealth > 0.0f || DamageResult.AppliedToArmor > 0.0f;
        Result.AppliedDamage = DamageResult.AppliedToHealth + DamageResult.AppliedToArmor;
        Result.bKilled = DamageResult.bKilled;
    }
    return Result;
}

bool UEmberWeaponComponent::BeginReload()
{
    if (!Definition || ReloadStage != EEmberReloadStage::None || MagazineAmmo >= Definition->MagazineCapacity || ReserveAmmo <= 0) return false;
    ReloadStage = EEmberReloadStage::Started;
    if (UWorld* World = GetWorld())
    {
        const float ReloadSeconds = MagazineAmmo == 0
            ? Definition->EmptyReloadSeconds : Definition->TacticalReloadSeconds;
        ReloadStartedAtSeconds = World->GetTimeSeconds();
        ReloadDurationSeconds = FMath::Max(0.1f, ReloadSeconds);
        World->GetTimerManager().SetTimer(ReloadTimer, this,
            &UEmberWeaponComponent::CompleteReload, ReloadDurationSeconds, false);
    }
    return true;
}

void UEmberWeaponComponent::AdvanceReloadStage(EEmberReloadStage NewStage)
{
    if (ReloadStage == EEmberReloadStage::None) return;
    ReloadStage = NewStage;
    if (NewStage == EEmberReloadStage::Completed) CompleteReload();
}

bool UEmberWeaponComponent::CancelReload()
{
    if (ReloadStage == EEmberReloadStage::None || ReloadStage == EEmberReloadStage::MagazineInserted) return false;
    ReloadStage = EEmberReloadStage::None;
    ReloadDurationSeconds = 0.0f;
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(ReloadTimer);
    return true;
}

void UEmberWeaponComponent::CompleteReload()
{
    const int32 Needed = Definition ? Definition->MagazineCapacity - MagazineAmmo : 0;
    const int32 Loaded = FMath::Min(Needed, ReserveAmmo);
    MagazineAmmo += Loaded;
    ReserveAmmo -= Loaded;
    ReloadStage = EEmberReloadStage::None;
    ReloadDurationSeconds = 0.0f;
    OnAmmoChanged.Broadcast(MagazineAmmo, ReserveAmmo);
}

float UEmberWeaponComponent::GetReloadProgress() const
{
    const UWorld* World = GetWorld();
    if (!World || ReloadStage == EEmberReloadStage::None || ReloadDurationSeconds <= 0.0f) return 0.0f;
    return FMath::Clamp(static_cast<float>((World->GetTimeSeconds() - ReloadStartedAtSeconds)
        / ReloadDurationSeconds), 0.0f, 1.0f);
}
