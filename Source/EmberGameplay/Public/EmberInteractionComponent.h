#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EmberInteractionComponent.generated.h"

UCLASS(ClassGroup=(Ember), meta=(BlueprintSpawnableComponent))
class EMBERGAMEPLAY_API UEmberInteractionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) bool TryInteract(const FVector& ViewOrigin, const FVector& ViewDirection);
protected:
    UPROPERTY(EditDefaultsOnly, Category="Interaction", meta=(ClampMin="10.0")) float InteractionRange = 250.0f;
};

