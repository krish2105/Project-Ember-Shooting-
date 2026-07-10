#include "EmberTacticalStateComponent.h"

UEmberTacticalStateComponent::UEmberTacticalStateComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UEmberTacticalStateComponent::SetState(EEmberAIState NewState)
{
    if (CurrentState == EEmberAIState::Dead || CurrentState == NewState) return false;
    const EEmberAIState Previous = CurrentState;
    CurrentState = NewState;
    OnStateChanged.Broadcast(Previous, CurrentState);
    return true;
}

void UEmberTacticalStateComponent::RememberThreat(AActor* Threat, const FVector& KnownLocation, float Confidence)
{
    CurrentThreat = Threat;
    LastKnownLocation = KnownLocation;
    ThreatConfidence = FMath::Clamp(Confidence, 0.0f, 1.0f);
    SetState(ThreatConfidence >= 0.75f ? EEmberAIState::Alerted : EEmberAIState::Suspicious);
}

void UEmberTacticalStateComponent::DecayThreat(float Amount)
{
    ThreatConfidence = FMath::Clamp(ThreatConfidence - FMath::Max(0.0f, Amount), 0.0f, 1.0f);
    if (ThreatConfidence <= 0.0f)
    {
        CurrentThreat.Reset();
        SetState(EEmberAIState::Searching);
    }
}

