#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EmberHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEmberHealthChanged, float, PreviousHealth, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEmberDeathEvent);

UCLASS(ClassGroup=(Ember), meta=(BlueprintSpawnableComponent))
class EMBERGAMEPLAY_API UEmberHealthComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UEmberHealthComponent();
    UFUNCTION(BlueprintCallable) float ApplyHealthDamage(float Amount);
    UFUNCTION(BlueprintCallable) void RestoreToFull();
    UFUNCTION(BlueprintCallable) void RestoreHealth(float Value);
    UFUNCTION(BlueprintPure) float GetHealth() const { return CurrentHealth; }
    UFUNCTION(BlueprintPure) float GetMaxHealth() const { return MaxHealth; }
    UFUNCTION(BlueprintPure) bool IsDead() const { return CurrentHealth <= 0.0f; }
    UPROPERTY(BlueprintAssignable) FEmberHealthChanged OnHealthChanged;
    UPROPERTY(BlueprintAssignable) FEmberDeathEvent OnDeath;
protected:
    UPROPERTY(EditDefaultsOnly, Category="Health", meta=(ClampMin="1.0")) float MaxHealth = 100.0f;
    UPROPERTY(VisibleInstanceOnly, Category="Health") float CurrentHealth = 100.0f;
};
