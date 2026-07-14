#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EmberPlayerController.generated.h"

UCLASS()
class EMBERGAMEPLAY_API AEmberPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

private:
    void ActivateGameplayInput();
};
