#include "EmberEnemyCharacter.h"
#include "EmberAIController.h"
#include "EmberArmorComponent.h"
#include "EmberDamageReceiverComponent.h"
#include "EmberHealthComponent.h"
#include "EmberTacticalStateComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
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
    Health->OnDeath.AddDynamic(this, &AEmberEnemyCharacter::HandleDeath);

    GetCharacterMovement()->MaxWalkSpeed = 360.0f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    bUseControllerRotationYaw = false;

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
        TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
    if (MeshAsset.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(MeshAsset.Object);
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }
    static ConstructorHelpers::FClassFinder<UAnimInstance> AnimClass(
        TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
    if (AnimClass.Succeeded()) GetMesh()->SetAnimInstanceClass(AnimClass.Class);
}

void AEmberEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    RestingMeshTransform = GetMesh()->GetRelativeTransform();
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

void AEmberEnemyCharacter::HandleDeath()
{
    GetWorldTimerManager().ClearTimer(HitReactionTimer);
    if (TacticalState) TacticalState->SetState(EEmberAIState::Dead);
    if (AController* ExistingController = GetController()) ExistingController->UnPossess();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    GetMesh()->SetSimulatePhysics(true);
    SetLifeSpan(12.0f);
}
