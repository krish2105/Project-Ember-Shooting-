#include "EmberAIController.h"
#include "EmberTacticalStateComponent.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

AEmberAIController::AEmberAIController()
{
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
    if (Stimulus.WasSuccessfullySensed()) Tactical->RememberThreat(Actor, Stimulus.StimulusLocation, 1.0f);
    else Tactical->DecayThreat(0.5f);
}
