#include "EmberEnemyCharacter.h"
#include "EmberAIController.h"
#include "EmberArmorComponent.h"
#include "EmberDamageReceiverComponent.h"
#include "EmberHealthComponent.h"
#include "EmberTacticalStateComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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

FEmberDamageResult AEmberEnemyCharacter::ReceiveEmberDamage_Implementation(const FEmberDamageSpec& DamageSpec)
{
    if (TacticalState) TacticalState->SetState(EEmberAIState::Suppressed);
    return DamageReceiver ? DamageReceiver->ApplyDamageSpec(DamageSpec) : FEmberDamageResult();
}

bool AEmberEnemyCharacter::IsAlive() const
{
    return Health && !Health->IsDead();
}

void AEmberEnemyCharacter::HandleDeath()
{
    if (TacticalState) TacticalState->SetState(EEmberAIState::Dead);
    if (AController* ExistingController = GetController()) ExistingController->UnPossess();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    GetMesh()->SetSimulatePhysics(true);
    SetLifeSpan(12.0f);
}
