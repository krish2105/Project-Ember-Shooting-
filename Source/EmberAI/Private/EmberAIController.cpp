#include "EmberAIController.h"
#include "EmberTacticalStateComponent.h"
#include "EmberInterfaces.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

AEmberAIController::AEmberAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    EmberPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("EmberPerception"));
    SetPerceptionComponent(*EmberPerception);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 3000.0f;
    SightConfig->LoseSightRadius = 3600.0f;
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
    if (!CombatTarget.IsValid() && LineOfSightTo(PlayerPawn)) CombatTarget = PlayerPawn;
    AActor* Target = CombatTarget.Get();
    if (!Target) return;

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
    MoveToLocation(Destination, 250.0f, true, true, false, true);

    if (Distance > 3600.0f || FireCooldown > 0.0f || !LineOfSightTo(Target)) return;
    const FVector Start = ControlledPawn->GetActorLocation() + FVector(0.0f, 0.0f, 65.0f);
    const FVector End = Target->GetActorLocation() + FVector(0.0f, 0.0f, 55.0f);
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(EmberAIFire), false, ControlledPawn);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params) && Hit.GetActor() == Target &&
        Target->GetClass()->ImplementsInterface(UEmberDamageable::StaticClass()))
    {
        FEmberDamageSpec Damage;
        Damage.BaseDamage = 8.0f;
        Damage.ShotDirection = (End - Start).GetSafeNormal();
        Damage.ImpactPoint = Hit.ImpactPoint;
        IEmberDamageable::Execute_ReceiveEmberDamage(Target, Damage);
        if (Tactical) Tactical->SetState(EEmberAIState::Engaging);
        FireCooldown = 0.55f + FMath::FRandRange(0.0f, 0.35f);
    }
}
