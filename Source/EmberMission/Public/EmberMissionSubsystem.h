#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EmberMissionDefinition.h"
#include "EmberTypes.h"
#include "EmberMissionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEmberObjectiveStateChanged, FName, ObjectiveId, EEmberObjectiveState, NewState);

UCLASS()
class EMBERMISSION_API UEmberMissionSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) bool StartMission(UEmberMissionDefinition* Definition);
    UFUNCTION(BlueprintCallable) bool SetObjectiveState(FName ObjectiveId, EEmberObjectiveState NewState);
    UFUNCTION(BlueprintPure) EEmberObjectiveState GetObjectiveState(FName ObjectiveId) const;
    UFUNCTION(BlueprintCallable) FEmberCheckpointSnapshot CaptureCheckpoint(FGuid CheckpointId) const;
    UFUNCTION(BlueprintCallable) bool RestoreCheckpoint(const FEmberCheckpointSnapshot& Snapshot);
    UPROPERTY(BlueprintAssignable) FEmberObjectiveStateChanged OnObjectiveStateChanged;
protected:
    UPROPERTY() TObjectPtr<UEmberMissionDefinition> ActiveMission;
    UPROPERTY() TMap<FName, EEmberObjectiveState> ObjectiveStates;
};
