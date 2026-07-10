#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EmberTypes.h"
#include "EmberBallisticsLibrary.generated.h"

UCLASS()
class EMBERGAMEPLAY_API UEmberBallisticsLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintPure, Category="Ember|Combat")
    static float CalculateDamage(const FEmberDamageSpec& Spec);

    UFUNCTION(BlueprintPure, Category="Ember|Combat")
    static float SecondsPerShot(float RoundsPerMinute);
};

