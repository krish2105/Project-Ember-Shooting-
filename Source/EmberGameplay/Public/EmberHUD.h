#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EmberHUD.generated.h"

UCLASS()
class EMBERGAMEPLAY_API AEmberHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};

