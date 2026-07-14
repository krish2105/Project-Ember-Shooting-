#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "EmberTypes.h"
#include "EmberMissionSaveGame.generated.h"

UCLASS()
class EMBERMISSION_API UEmberMissionSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY() FEmberCheckpointSnapshot Snapshot;
};
