#include "EmberAIController.h"
#include "EmberEnemyCharacter.h"
#include "EmberTacticalStateComponent.h"
#include "EmberInterfaces.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Navigation/PathFollowingComponent.h"
#include "EmberLog.h"

AEmberAIController::AEmberAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    EmberPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("EmberPerception"));
    SetPerceptionComponent(*EmberPerception);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 6500.0f;
    SightConfig->LoseSightRadius = 7500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 70.0f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 4000.0f;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

    EmberPerception->ConfigureSense(*SightConfig);
    EmberPerception->ConfigureSense(*HearingConfig);
    EmberPerception->SetDominantSense(SightConfig->GetSenseImplementation());
    EmberPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AEmberAIController::HandleTargetPerceptionUpdated);
}

void AEmberAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    PatrolAnchor = InPawn ? InPawn->GetActorLocation() : FVector::ZeroVector;
    PatrolAngle = FMath::FRandRange(0.0f, TWO_PI);
    if (InPawn && !InPawn->FindComponentByClass<UEmberTacticalStateComponent>())
    {
        UE_LOG(LogTemp, Warning, TEXT("AI pawn %s has no Ember tactical state component."), *InPawn->GetName());
    }
}

void AEmberAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    APawn* ControlledPawn = GetPawn();
    UEmberTacticalStateComponent* Tactical = ControlledPawn ? ControlledPawn->FindComponentByClass<UEmberTacticalStateComponent>() : nullptr;
    if (!Tactical || !Actor) return;
    if (Stimulus.WasSuccessfullySensed())
    {
        CombatTarget = Actor;
        Tactical->RememberThreat(Actor, Stimulus.StimulusLocation, 1.0f);
    }
    else Tactical->DecayThreat(0.5f);
}

void AEmberAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    DecisionAccumulator += DeltaSeconds;
    FireCooldown = FMath::Max(0.0f, FireCooldown - DeltaSeconds);
    if (DecisionAccumulator < 0.2f) return;
    DecisionAccumulator = 0.0f;

    APawn* ControlledPawn = GetPawn();
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!ControlledPawn || !PlayerPawn) return;
    const float PlayerDistance = FVector::Dist2D(ControlledPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (!CombatTarget.IsValid() && (PlayerDistance < 7000.0f || LineOfSightTo(PlayerPawn))) CombatTarget = PlayerPawn;
    AActor* Target = CombatTarget.Get();
    if (!Target)
    {
        PatrolAngle += 0.12f;
        const FVector PatrolGoal = PatrolAnchor + FVector(FMath::Cos(PatrolAngle), FMath::Sin(PatrolAngle), 0.0f) * 420.0f;
        const EPathFollowingRequestResult::Type Result = MoveToLocation(PatrolGoal, 90.0f, true, true, false, true);
        if (Result == EPathFollowingRequestResult::Failed)
            ControlledPawn->AddMovementInput((PatrolGoal - ControlledPawn->GetActorLocation()).GetSafeNormal2D(), 0.65f);
        return;
    }
    SetFocus(Target);

    UEmberTacticalStateComponent* Tactical = ControlledPawn->FindComponentByClass<UEmberTacticalStateComponent>();
    const FVector ToTarget = Target->GetActorLocation() - ControlledPawn->GetActorLocation();
    const float Distance = ToTarget.Size2D();
    FVector Destination = Target->GetActorLocation();
    if (Distance < 850.0f)
    {
        if (Tactical) Tactical->SetState(EEmberAIState::Retreating);
        Destination = ControlledPawn->GetActorLocation() - ToTarget.GetSafeNormal2D() * 600.0f;
    }
    else if (Distance < 2200.0f)
    {
        if (Tactical) Tactical->SetState(EEmberAIState::Flanking);
        Destination += FVector::CrossProduct(ToTarget.GetSafeNormal2D(), FVector::UpVector) * 650.0f;
    }
    else
    {
        if (Tactical) Tactical->SetState(EEmberAIState::Advancing);
    }
    const EPathFollowingRequestResult::Type MoveResult = MoveToLocation(Destination, 250.0f, true, true, false, true);
    if (MoveResult == EPathFollowingRequestResult::Failed || ControlledPawn->GetVelocity().SizeSquared2D() < 25.0f)
        ControlledPawn->AddMovementInput((Destination - ControlledPawn->GetActorLocation()).GetSafeNormal2D(), 1.0f);

    // FireAt performs the authoritative muzzle visibility trace. Using
    // AAIController::LineOfSightTo here as a second gate prevented suppression
    // fire in the generated World Partition map when the target was visible
    // from the muzzle but not from the controller's abstract eye point.
    if (Distance > 4500.0f || FireCooldown > 0.0f) return;
    if (AEmberEnemyCharacter* Enemy = Cast<AEmberEnemyCharacter>(ControlledPawn))
    {
        Enemy->FireAt(Target);
        if (Tactical) Tactical->SetState(EEmberAIState::Engaging);
        FireCooldown = 0.65f + FMath::FRandRange(0.0f, 0.45f);
    }
}
