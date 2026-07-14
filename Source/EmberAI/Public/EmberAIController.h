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
    /** Tactical ring used to keep combatants readable in the third-person camera. */
    static constexpr float MinimumCombatRange = 1150.0f;
    static constexpr float DesiredCombatRange = 1850.0f;
    static constexpr float MaximumCombatRange = 2850.0f;
    static constexpr float MinimumFireRange = 900.0f;
protected:
    virtual void OnPossess(APawn* InPawn) override;
    UFUNCTION() void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
    UPROPERTY(VisibleAnywhere) TObjectPtr<UAIPerceptionComponent> EmberPerception;
    UPROPERTY() TObjectPtr<UAISenseConfig_Sight> SightConfig;
    UPROPERTY() TObjectPtr<UAISenseConfig_Hearing> HearingConfig;
    UPROPERTY() TWeakObjectPtr<AActor> CombatTarget;
    FVector PatrolAnchor = FVector::ZeroVector;
    float DecisionAccumulator = 0.0f;
    float FireCooldown = 0.0f;
    float PatrolAngle = 0.0f;
    float FlankDirection = 1.0f;
    bool bFacingAlignmentLogged = false;
};
