#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EmberTypes.h"
#include "EmberDamageReceiverComponent.generated.h"

class UEmberArmorComponent;
class UEmberHealthComponent;

UCLASS(ClassGroup=(Ember), meta=(BlueprintSpawnableComponent))
class EMBERGAMEPLAY_API UEmberDamageReceiverComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) FEmberDamageResult ApplyDamageSpec(const FEmberDamageSpec& Spec);
};

