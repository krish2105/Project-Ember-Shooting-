#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "EmberBootstrapDataCommandlet.generated.h"

UCLASS()
class UEmberBootstrapDataCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    UEmberBootstrapDataCommandlet();
    virtual int32 Main(const FString& Params) override;
};

