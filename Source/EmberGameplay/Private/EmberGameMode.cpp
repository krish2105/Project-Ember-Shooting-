#include "EmberGameMode.h"
#include "EmberCharacter.h"
#include "EmberHUD.h"
#include "EmberPlayerController.h"

AEmberGameMode::AEmberGameMode()
{
    DefaultPawnClass = AEmberCharacter::StaticClass();
    PlayerControllerClass = AEmberPlayerController::StaticClass();
    HUDClass = AEmberHUD::StaticClass();
}
