#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EmberTacticalStateComponent.generated.h"

UENUM(BlueprintType)
enum class EEmberAIState : uint8
{
    Relaxed, Patrol, Suspicious, Investigating, Alerted, SeekingCover, Engaging,
    Suppressed, Flanking, Advancing, Retreating, Searching, CallingReinforcement,
    Stunned, Wounded, Dead
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEmberAIStateChanged, EEmberAIState, PreviousState, EEmberAIState, NewState);

UCLASS(ClassGroup=(Ember), meta=(BlueprintSpawnableComponent))
class EMBERAI_API UEmberTacticalStateComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UEmberTacticalStateComponent();
    UFUNCTION(BlueprintCallable) bool SetState(EEmberAIState NewState);
    UFUNCTION(BlueprintCallable) void RememberThreat(AActor* Threat, const FVector& KnownLocation, float Confidence);
    UFUNCTION(BlueprintCallable) void DecayThreat(float Amount);
    UFUNCTION(BlueprintPure) EEmberAIState GetState() const { return CurrentState; }
    UFUNCTION(BlueprintPure) FVector GetLastKnownLocation() const { return LastKnownLocation; }
    UFUNCTION(BlueprintPure) float GetThreatConfidence() const { return ThreatConfidence; }
    UPROPERTY(BlueprintAssignable) FEmberAIStateChanged OnStateChanged;
protected:
    UPROPERTY(VisibleInstanceOnly) EEmberAIState CurrentState = EEmberAIState::Relaxed;
    UPROPERTY(VisibleInstanceOnly) TWeakObjectPtr<AActor> CurrentThreat;
    UPROPERTY(VisibleInstanceOnly) FVector LastKnownLocation = FVector::ZeroVector;
    UPROPERTY(VisibleInstanceOnly) float ThreatConfidence = 0.0f;
};

