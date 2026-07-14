#include "EmberEnemyCharacter.h"
#include "EmberAIController.h"
#include "EmberArmorComponent.h"
#include "EmberDamageReceiverComponent.h"
#include "EmberHealthComponent.h"
#include "EmberTacticalStateComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Sound/SoundWaveProcedural.h"
#include "EmberLog.h"
#include "UObject/ConstructorHelpers.h"

AEmberEnemyCharacter::AEmberEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    AIControllerClass = AEmberAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    Tags.Add(TEXT("EmberEnemy"));

    Health = CreateDefaultSubobject<UEmberHealthComponent>(TEXT("Health"));
    Armor = CreateDefaultSubobject<UEmberArmorComponent>(TEXT("Armor"));
    DamageReceiver = CreateDefaultSubobject<UEmberDamageReceiverComponent>(TEXT("DamageReceiver"));
    TacticalState = CreateDefaultSubobject<UEmberTacticalStateComponent>(TEXT("TacticalState"));
    HostileMarker = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HostileMarker"));
    HostileMarker->SetupAttachment(RootComponent);
    HostileMarker->SetRelativeLocation(FVector(0.0f, 0.0f, 125.0f));
    HostileMarker->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    HostileMarker->SetText(FText::FromString(TEXT("HOSTILE")));
    HostileMarker->SetTextRenderColor(FColor(255, 35, 20));
    HostileMarker->SetHorizontalAlignment(EHTA_Center);
    HostileMarker->SetWorldSize(24.0f);
    HostileMarker->SetVisibility(false);
    HostileMarker->SetHiddenInGame(true);
    Health->OnDeath.AddDynamic(this, &AEmberEnemyCharacter::HandleDeath);

    GetCharacterMovement()->MaxWalkSpeed = 360.0f;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    bUseControllerRotationYaw = true;

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
        TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
    if (MeshAsset.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(MeshAsset.Object);
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }
    static ConstructorHelpers::FClassFinder<UAnimInstance> AnimClass(
        TEXT("/Game/Variant_Shooter/Anims/ABP_TP_Rifle"));
    if (AnimClass.Succeeded()) GetMesh()->SetAnimInstanceClass(AnimClass.Class);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> RifleMesh(
        TEXT("/Game/Weapons/Rifle/Meshes/SM_Rifle.SM_Rifle"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    WeaponVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyWeaponVisual"));
    WeaponVisual->SetupAttachment(GetMesh(), TEXT("HandGrip_R"));
    WeaponVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponVisual->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
    WeaponVisual->SetRelativeScale3D(FVector(1.08f));
    if (RifleMesh.Succeeded()) WeaponVisual->SetStaticMesh(RifleMesh.Object);

    MuzzleFlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("EnemyMuzzleFlash"));
    MuzzleFlashLight->SetupAttachment(WeaponVisual);
    MuzzleFlashLight->SetRelativeLocation(FVector(0.0f, 58.0f, 7.0f));
    MuzzleFlashLight->SetLightColor(FLinearColor(1.0f, 0.22f, 0.025f));
    MuzzleFlashLight->SetAttenuationRadius(420.0f);
    MuzzleFlashLight->SetIntensity(0.0f);

    ShotTracerVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyShotTracer"));
    ShotTracerVisual->SetupAttachment(RootComponent);
    ShotTracerVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ShotTracerVisual->SetCastShadow(false);
    ShotTracerVisual->SetVisibility(false);
    ShotTracerVisual->SetHiddenInGame(true);
    if (CubeMesh.Succeeded()) ShotTracerVisual->SetStaticMesh(CubeMesh.Object);

    GunshotAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EnemyGunshotAudio"));
    GunshotAudio->SetupAttachment(RootComponent);
    GunshotAudio->bAutoActivate = false;
    GunshotAudio->bAllowSpatialization = true;
}

void AEmberEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    // Placed World Partition actors can retain an older serialized possession
    // setting after the native class changes. Guarantee that every live hostile
    // receives its tactical controller in both Editor and packaged builds.
    if (!GetController()) SpawnDefaultController();
    RestingMeshTransform = GetMesh()->GetRelativeTransform();
    InitializeGunshotAudio();
    UE_LOG(LogEmberAI, Log, TEXT("Enemy combatant ready: %s controller=%s"),
        *GetName(), GetController() ? *GetController()->GetClass()->GetName() : TEXT("NONE"));
}

FEmberDamageResult AEmberEnemyCharacter::ReceiveEmberDamage_Implementation(const FEmberDamageSpec& DamageSpec)
{
    if (TacticalState) TacticalState->SetState(EEmberAIState::Suppressed);
    const FEmberDamageResult Result = DamageReceiver
        ? DamageReceiver->ApplyDamageSpec(DamageSpec) : FEmberDamageResult();
    if (!Result.bKilled && Result.AppliedToHealth + Result.AppliedToArmor > 0.0f)
    {
        PlayHitReaction(DamageSpec);
    }
    return Result;
}

void AEmberEnemyCharacter::PlayHitReaction(const FEmberDamageSpec& DamageSpec)
{
    if (!GetMesh()) return;
    GetWorldTimerManager().ClearTimer(HitReactionTimer);
    GetMesh()->SetRelativeTransform(RestingMeshTransform);
    const FVector LocalIncoming = GetActorTransform().InverseTransformVectorNoScale(
        FVector(DamageSpec.ShotDirection).GetSafeNormal());
    const float ReactionStrength = FMath::Clamp(3.0f + DamageSpec.Stagger * 0.08f, 3.0f, 9.0f);
    GetMesh()->AddLocalRotation(FRotator(
        -LocalIncoming.Z * ReactionStrength,
        0.0f,
        LocalIncoming.Y * ReactionStrength));
    GetWorldTimerManager().SetTimer(HitReactionTimer, this,
        &AEmberEnemyCharacter::ResetHitReaction, 0.12f, false);
}

void AEmberEnemyCharacter::ResetHitReaction()
{
    if (GetMesh() && IsAlive()) GetMesh()->SetRelativeTransform(RestingMeshTransform);
}

bool AEmberEnemyCharacter::IsAlive() const
{
    return Health && !Health->IsDead();
}

bool AEmberEnemyCharacter::FireAt(AActor* Target)
{
    if (!IsAlive() || !Target || !GetWorld() || !MuzzleFlashLight) return false;
    const FVector Start = MuzzleFlashLight->GetComponentLocation();
    const FVector TraceStart = GetActorLocation() + FVector(0.0f, 0.0f, 68.0f);
    const FVector AimPoint = Target->GetActorLocation() + FVector(0.0f, 0.0f, 55.0f);
    const float Distance = FVector::Distance(Start, AimPoint);
    const float Spread = Distance > 2200.0f ? 0.035f : 0.018f;
    const FVector Direction = FMath::VRandCone((AimPoint - TraceStart).GetSafeNormal(), Spread);
    const FVector End = TraceStart + Direction * FMath::Max(4500.0f, Distance + 300.0f);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(EmberEnemyRifle), true, this);
    FHitResult Hit;
    GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, End, ECC_Visibility, Params);
    const FVector TraceEnd = Hit.bBlockingHit ? Hit.ImpactPoint : End;

    if (ShotTracerVisual)
    {
        const FVector Delta = TraceEnd - Start;
        ShotTracerVisual->SetWorldLocation(Start + Delta * 0.5f);
        ShotTracerVisual->SetWorldRotation(Delta.Rotation());
        ShotTracerVisual->SetWorldScale3D(FVector(Delta.Size() / 100.0f, 0.018f, 0.018f));
        ShotTracerVisual->SetHiddenInGame(false);
        ShotTracerVisual->SetVisibility(true);
        GetWorldTimerManager().ClearTimer(ShotTracerTimer);
        GetWorldTimerManager().SetTimer(ShotTracerTimer, this,
            &AEmberEnemyCharacter::ResetShotTracer, 0.07f, false);
    }
    MuzzleFlashLight->SetIntensity(11000.0f);
    GetWorldTimerManager().ClearTimer(MuzzleFlashTimer);
    GetWorldTimerManager().SetTimer(MuzzleFlashTimer, this,
        &AEmberEnemyCharacter::ResetMuzzleFlash, 0.055f, false);
    PlayGunshotAudio();

    AActor* HitActor = Hit.GetActor();
    // Character capsules in some generated/template collision presets do not
    // block Visibility. An unobstructed near-miss therefore uses the weapon's
    // accuracy roll against the intended target; a world or friendly blocker
    // still stops the shot and prevents damage through cover.
    AActor* DamageActor = HitActor;
    FVector DamagePoint = Hit.ImpactPoint;
    if (!Hit.bBlockingHit && FMath::FRand() < 0.42f)
    {
        DamageActor = Target;
        DamagePoint = AimPoint;
    }
    if (DamageActor && DamageActor->GetClass()->ImplementsInterface(UEmberDamageable::StaticClass()))
    {
        FEmberDamageSpec Damage;
        Damage.BaseDamage = 14.0f;
        Damage.ArmorModifier = 1.0f;
        Damage.Stagger = 8.0f;
        Damage.Suppression = 12.0f;
        Damage.ShotDirection = Direction;
        Damage.ImpactPoint = DamagePoint;
        const FEmberDamageResult Result = IEmberDamageable::Execute_ReceiveEmberDamage(DamageActor, Damage);
        UE_LOG(LogEmberAI, Log, TEXT("Enemy rifle fired: hit=%s health=%.1f armor=%.1f"),
            DamageActor == Target ? TEXT("TARGET") : TEXT("OTHER"), Result.AppliedToHealth, Result.AppliedToArmor);
        return DamageActor == Target;
    }
    UE_LOG(LogEmberAI, Log, TEXT("Enemy rifle fired: %s"),
        Hit.bBlockingHit ? TEXT("BLOCKED") : TEXT("OPEN_MISS"));
    return false;
}

void AEmberEnemyCharacter::InitializeGunshotAudio()
{
    if (!GunshotAudio || GunshotWave) return;
    GunshotWave = NewObject<USoundWaveProcedural>(this, TEXT("EnemyRuntimeGunshot"));
    GunshotWave->SetSampleRate(22050);
    GunshotWave->NumChannels = 1;
    GunshotWave->Duration = 0.1f;
    GunshotWave->SoundGroup = SOUNDGROUP_Effects;
    constexpr int32 SampleCount = 2205;
    GunshotPCM.SetNumUninitialized(SampleCount * sizeof(int16));
    int16* Samples = reinterpret_cast<int16*>(GunshotPCM.GetData());
    FRandomStream Noise(GetUniqueID());
    for (int32 Index = 0; Index < SampleCount; ++Index)
    {
        const float T = static_cast<float>(Index) / 22050.0f;
        const float Envelope = FMath::Exp(-T * 39.0f);
        const float Crack = Noise.FRandRange(-1.0f, 1.0f);
        const float Report = FMath::Sin(2.0f * PI * 118.0f * T);
        Samples[Index] = static_cast<int16>(
            FMath::Clamp((Crack * 0.8f + Report * 0.2f) * Envelope, -1.0f, 1.0f) * 22000.0f);
    }
    GunshotAudio->SetSound(GunshotWave);
}

void AEmberEnemyCharacter::PlayGunshotAudio()
{
    if (!GunshotWave || !GunshotAudio || GunshotPCM.IsEmpty()) return;
    GunshotWave->ResetAudio();
    GunshotWave->QueueAudio(GunshotPCM.GetData(), GunshotPCM.Num());
    GunshotAudio->Stop();
    GunshotAudio->Play();
}

void AEmberEnemyCharacter::ResetMuzzleFlash()
{
    if (MuzzleFlashLight) MuzzleFlashLight->SetIntensity(0.0f);
}

void AEmberEnemyCharacter::ResetShotTracer()
{
    if (ShotTracerVisual)
    {
        ShotTracerVisual->SetVisibility(false);
        ShotTracerVisual->SetHiddenInGame(true);
    }
}

void AEmberEnemyCharacter::HandleDeath()
{
    GetWorldTimerManager().ClearTimer(HitReactionTimer);
    GetWorldTimerManager().ClearTimer(MuzzleFlashTimer);
    GetWorldTimerManager().ClearTimer(ShotTracerTimer);
    ResetMuzzleFlash();
    ResetShotTracer();
    if (TacticalState) TacticalState->SetState(EEmberAIState::Dead);
    if (AController* ExistingController = GetController()) ExistingController->UnPossess();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    GetMesh()->SetSimulatePhysics(true);
    SetLifeSpan(12.0f);
}
