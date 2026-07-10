#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "EmberDeveloperSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="PROJECT EMBER"))
class EMBERCORE_API UEmberDeveloperSettings : public UDeveloperSettings
{
    GENERATED_BODY()
public:
    UPROPERTY(Config, EditAnywhere, Category="Save", meta=(ClampMin="1")) int32 SaveSchemaVersion = 1;
    UPROPERTY(Config, EditAnywhere, Category="Budgets", meta=(ClampMin="1")) int32 MaxActiveProjectiles = 128;
    UPROPERTY(Config, EditAnywhere, Category="Budgets", meta=(ClampMin="1")) int32 MaxPersistentDecals = 256;
    UPROPERTY(Config, EditAnywhere, Category="Budgets", meta=(ClampMin="0")) int32 MaxShellCasings = 48;
    UPROPERTY(Config, EditAnywhere, Category="AI", meta=(ClampMin="1")) int32 BaselineMaxAI = 12;
    UPROPERTY(Config, EditAnywhere, Category="AI", meta=(ClampMin="1")) int32 HighMaxAI = 20;
    UPROPERTY(Config, EditAnywhere, Category="AI", meta=(ClampMin="1")) int32 UltraMaxAI = 28;
};

