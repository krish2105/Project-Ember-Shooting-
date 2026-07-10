#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "EmberValidateContentCommandlet.generated.h"

UCLASS()
class UEmberValidateContentCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    UEmberValidateContentCommandlet();
    virtual int32 Main(const FString& Params) override;
};

