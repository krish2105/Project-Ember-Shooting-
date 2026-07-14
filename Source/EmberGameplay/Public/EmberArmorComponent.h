#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EmberArmorComponent.generated.h"

UCLASS(ClassGroup=(Ember), meta=(BlueprintSpawnableComponent))
class EMBERGAMEPLAY_API UEmberArmorComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UEmberArmorComponent();
    UFUNCTION(BlueprintCallable) float AbsorbDamage(float IncomingDamage, float ArmorModifier, float& OutAbsorbed);
    UFUNCTION(BlueprintPure) float GetArmor() const { return CurrentArmor; }
    UFUNCTION(BlueprintCallable) void RestoreArmor(float Value);
protected:
    UPROPERTY(EditDefaultsOnly, Category="Armor", meta=(ClampMin="0.0")) float MaxArmor = 100.0f;
    /** Portion of incoming damage intercepted while armor remains. The rest
        reaches health so every hostile hit has clear gameplay feedback. */
    UPROPERTY(EditDefaultsOnly, Category="Armor", meta=(ClampMin="0.0", ClampMax="1.0")) float ProtectionRatio = 0.6f;
    UPROPERTY(VisibleInstanceOnly, Category="Armor") float CurrentArmor = 100.0f;
};
