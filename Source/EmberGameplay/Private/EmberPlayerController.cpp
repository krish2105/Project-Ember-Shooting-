#include "EmberPlayerController.h"
#include "EmberLog.h"
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
    // Ignore-input APIs are stack based. BeginPlay and OnPossess can both run
    // during packaged startup, so normalize before adding the short launch
    // guard instead of leaving two locks for one delayed unlock.
    ResetIgnoreLookInput();
    ResetIgnoreMoveInput();
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
    ResetIgnoreLookInput();
    ResetIgnoreMoveInput();
    SetInputMode(FInputModeGameOnly());
    UE_LOG(LogEmberCombat, Log, TEXT("Gameplay mouse/look input activated; ignored=%s"),
        IsLookInputIgnored() ? TEXT("true") : TEXT("false"));
}
