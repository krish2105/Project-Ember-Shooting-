#include "EmberMissionSubsystem.h"
#include "EmberDeveloperSettings.h"
#include "EmberArmorComponent.h"
#include "EmberCharacter.h"
#include "EmberHealthComponent.h"
#include "Kismet/GameplayStatics.h"

bool UEmberMissionSubsystem::StartMission(UEmberMissionDefinition* Definition)
{
    if (!IsValid(Definition) || Definition->Identifier.IsNone() || Definition->Objectives.IsEmpty()) return false;
    ActiveMission = Definition;
    ObjectiveStates.Reset();
    for (const FEmberObjectiveDefinition& Objective : Definition->Objectives)
    {
        ObjectiveStates.Add(Objective.Identifier, Objective.Prerequisites.IsEmpty() ? EEmberObjectiveState::Active : EEmberObjectiveState::Inactive);
    }
    return true;
}

bool UEmberMissionSubsystem::SetObjectiveState(FName ObjectiveId, EEmberObjectiveState NewState)
{
    EEmberObjectiveState* Existing = ObjectiveStates.Find(ObjectiveId);
    if (!Existing || *Existing == NewState) return false;
    *Existing = NewState;
    OnObjectiveStateChanged.Broadcast(ObjectiveId, NewState);
    if (NewState == EEmberObjectiveState::Completed && ActiveMission)
    {
        for (const FEmberObjectiveDefinition& Objective : ActiveMission->Objectives)
        {
            EEmberObjectiveState* Candidate = ObjectiveStates.Find(Objective.Identifier);
            if (!Candidate || *Candidate != EEmberObjectiveState::Inactive) continue;
            const bool bReady = Objective.Prerequisites.IsEmpty() || Objective.Prerequisites.ContainsByPredicate(
                [this](FName Prerequisite) { return GetObjectiveState(Prerequisite) != EEmberObjectiveState::Completed; }) == false;
            if (bReady)
            {
                *Candidate = EEmberObjectiveState::Active;
                OnObjectiveStateChanged.Broadcast(Objective.Identifier, EEmberObjectiveState::Active);
            }
        }
    }
    return true;
}

EEmberObjectiveState UEmberMissionSubsystem::GetObjectiveState(FName ObjectiveId) const
{
    const EEmberObjectiveState* State = ObjectiveStates.Find(ObjectiveId);
    return State ? *State : EEmberObjectiveState::Inactive;
}

FEmberCheckpointSnapshot UEmberMissionSubsystem::CaptureCheckpoint(FGuid CheckpointId) const
{
    FEmberCheckpointSnapshot Snapshot;
    Snapshot.SchemaVersion = GetDefault<UEmberDeveloperSettings>()->SaveSchemaVersion;
    Snapshot.CheckpointId = CheckpointId;
    if (GetWorld())
    {
        if (AEmberCharacter* Player = Cast<AEmberCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
        {
            Snapshot.PlayerTransform = Player->GetActorTransform();
            if (UEmberHealthComponent* Health = Player->GetHealthComponent()) Snapshot.Health = Health->GetHealth();
            if (UEmberArmorComponent* Armor = Player->GetArmorComponent()) Snapshot.Armor = Armor->GetArmor();
            Player->WriteWeaponCheckpoint(Snapshot);
        }
    }
    for (const TPair<FName, EEmberObjectiveState>& Pair : ObjectiveStates)
    {
        Snapshot.ObjectiveStates.Add(Pair.Key, static_cast<uint8>(Pair.Value));
    }
    return Snapshot;
}

bool UEmberMissionSubsystem::RestoreCheckpoint(const FEmberCheckpointSnapshot& Snapshot)
{
    if (!Snapshot.CheckpointId.IsValid() || Snapshot.SchemaVersion != GetDefault<UEmberDeveloperSettings>()->SaveSchemaVersion) return false;
    for (const TPair<FName, uint8>& Pair : Snapshot.ObjectiveStates)
    {
        if (!ObjectiveStates.Contains(Pair.Key) || Pair.Value > static_cast<uint8>(EEmberObjectiveState::Failed)) return false;
    }
    for (const TPair<FName, uint8>& Pair : Snapshot.ObjectiveStates)
    {
        ObjectiveStates[Pair.Key] = static_cast<EEmberObjectiveState>(Pair.Value);
        OnObjectiveStateChanged.Broadcast(Pair.Key, ObjectiveStates[Pair.Key]);
    }
    if (GetWorld())
    {
        if (AEmberCharacter* Player = Cast<AEmberCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
        {
            Player->SetActorTransform(Snapshot.PlayerTransform, false, nullptr, ETeleportType::TeleportPhysics);
            if (UEmberHealthComponent* Health = Player->GetHealthComponent()) Health->RestoreHealth(Snapshot.Health);
            if (UEmberArmorComponent* Armor = Player->GetArmorComponent()) Armor->RestoreArmor(Snapshot.Armor);
            if (!Snapshot.AmmunitionByType.IsEmpty()) Player->RestoreWeaponCheckpoint(Snapshot);
        }
    }
    return true;
}
