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
protected:
    UPROPERTY(EditDefaultsOnly, Category="Armor", meta=(ClampMin="0.0")) float MaxArmor = 100.0f;
    UPROPERTY(VisibleInstanceOnly, Category="Armor") float CurrentArmor = 100.0f;
};

