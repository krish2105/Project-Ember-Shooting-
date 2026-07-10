#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EmberAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Hearing;
class UAISenseConfig_Sight;
class UEmberTacticalStateComponent;

UCLASS()
class EMBERAI_API AEmberAIController : public AAIController
{
    GENERATED_BODY()
public:
    AEmberAIController();
    virtual void Tick(float DeltaSeconds) override;
protected:
    virtual void OnPossess(APawn* InPawn) override;
    UFUNCTION() void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
    UPROPERTY(VisibleAnywhere) TObjectPtr<UAIPerceptionComponent> EmberPerception;
    UPROPERTY() TObjectPtr<UAISenseConfig_Sight> SightConfig;
    UPROPERTY() TObjectPtr<UAISenseConfig_Hearing> HearingConfig;
    UPROPERTY() TWeakObjectPtr<AActor> CombatTarget;
    float DecisionAccumulator = 0.0f;
    float FireCooldown = 0.0f;
};
