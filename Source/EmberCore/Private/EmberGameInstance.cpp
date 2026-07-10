#include "EmberGameInstance.h"
#include "EmberLog.h"

void UEmberGameInstance::Init()
{
    Super::Init();
    UE_LOG(LogEmber, Log, TEXT("PROJECT EMBER game instance initialized."));
}
