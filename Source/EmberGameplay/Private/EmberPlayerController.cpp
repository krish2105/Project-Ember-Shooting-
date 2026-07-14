#include "EmberPlayerController.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

void AEmberPlayerController::BeginPlay()
{
    Super::BeginPlay();
    ArmGameplayInput();
}

void AEmberPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    ArmGameplayInput();
}

void AEmberPlayerController::ArmGameplayInput()
{
    SetPause(false);
    bShowMouseCursor = false;
    SetIgnoreLookInput(true);
    SetIgnoreMoveInput(false);
    SetInputMode(FInputModeGameOnly());
    GetWorldTimerManager().ClearTimer(GameplayInputActivationTimer);
    GetWorldTimerManager().SetTimer(
        GameplayInputActivationTimer,
        this,
        &AEmberPlayerController::ActivateGameplayInput,
        0.25f,
        false);
}

void AEmberPlayerController::ActivateGameplayInput()
{
    if (const APawn* ControlledPawn = GetPawn())
    {
        SetControlRotation(FRotator(0.0f, ControlledPawn->GetActorRotation().Yaw, 0.0f));
    }
    SetPause(false);
    bShowMouseCursor = false;
    SetIgnoreLookInput(false);
    SetIgnoreMoveInput(false);
    SetInputMode(FInputModeGameOnly());
}
