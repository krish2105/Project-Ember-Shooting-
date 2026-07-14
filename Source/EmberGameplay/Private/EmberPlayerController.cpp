#include "EmberPlayerController.h"
#include "GameFramework/Pawn.h"

void AEmberPlayerController::BeginPlay()
{
    Super::BeginPlay();
    ActivateGameplayInput();
}

void AEmberPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    ActivateGameplayInput();
}

void AEmberPlayerController::ActivateGameplayInput()
{
    SetPause(false);
    bShowMouseCursor = false;
    SetIgnoreLookInput(false);
    SetIgnoreMoveInput(false);
    SetInputMode(FInputModeGameOnly());
}
