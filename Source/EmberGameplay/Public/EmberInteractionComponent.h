#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EmberInteractionComponent.generated.h"

class APawn;

UCLASS(ClassGroup=(Ember), meta=(BlueprintSpawnableComponent))
class EMBERGAMEPLAY_API UEmberInteractionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) bool TryInteract(const FVector& ViewOrigin, const FVector& ViewDirection);
    UFUNCTION(BlueprintCallable) APawn* FindDriveableVehicle(const FVector& ViewOrigin, const FVector& ViewDirection) const;
protected:
    UPROPERTY(EditDefaultsOnly, Category="Interaction", meta=(ClampMin="10.0")) float InteractionRange = 450.0f;
};
