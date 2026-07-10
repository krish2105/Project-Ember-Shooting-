#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EmberGameInstance.generated.h"

UCLASS()
class EMBERCORE_API UEmberGameInstance : public UGameInstance
{
    GENERATED_BODY()
public:
    virtual void Init() override;
};

