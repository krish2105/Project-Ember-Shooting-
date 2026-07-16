#include "EmberHealthComponent.h"

UEmberHealthComponent::UEmberHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

float UEmberHealthComponent::ApplyHealthDamage(float Amount)
{
    if (Amount <= 0.0f || IsDead()) return 0.0f;
    const float Previous = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.0f, MaxHealth);
    OnHealthChanged.Broadcast(Previous, CurrentHealth);
    if (CurrentHealth <= 0.0f) OnDeath.Broadcast();
    return Previous - CurrentHealth;
}

void UEmberHealthComponent::RestoreToFull()
{
    const float Previous = CurrentHealth;
    CurrentHealth = MaxHealth;
    OnHealthChanged.Broadcast(Previous, CurrentHealth);
}

void UEmberHealthComponent::RestoreHealth(float Value)
{
    const float Previous = CurrentHealth;
    CurrentHealth = FMath::Clamp(Value, 0.0f, MaxHealth);
    OnHealthChanged.Broadcast(Previous, CurrentHealth);
}
