#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "EmberImportThirdPersonCommandlet.generated.h"

UCLASS()
class UEmberImportThirdPersonCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    UEmberImportThirdPersonCommandlet();
    virtual int32 Main(const FString& Params) override;
};

