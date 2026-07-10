#include "EmberGameMode.h"
#include "EmberCharacter.h"
#include "EmberHUD.h"

AEmberGameMode::AEmberGameMode()
{
    DefaultPawnClass = AEmberCharacter::StaticClass();
    HUDClass = AEmberHUD::StaticClass();
}

