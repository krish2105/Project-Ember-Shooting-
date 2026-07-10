#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EmberTypes.h"
#include "EmberMissionDirector.generated.h"

class UEmberMissionDefinition;

UCLASS()
class EMBERMISSION_API AEmberMissionDirector : public AActor
{
    GENERATED_BODY()
public:
    AEmberMissionDirector();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UFUNCTION(BlueprintPure) int32 GetRemainingEnemies() const { return RemainingEnemies; }
    UFUNCTION(BlueprintPure) bool IsMissionComplete() const { return bMissionComplete; }
protected:
    void Complete(FName ObjectiveId);
    void UpdateMission();
    UPROPERTY() TObjectPtr<UEmberMissionDefinition> MissionDefinition;
    FVector LastCheckpointLocation = FVector(-45000.0f, 0.0f, 250.0f);
    int32 RemainingEnemies = 0;
    float UpdateAccumulator = 0.0f;
    bool bCheckpointReached = false;
    bool bMissionComplete = false;
};
