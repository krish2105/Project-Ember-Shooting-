#include "Misc/AutomationTest.h"
#include "EmberMissionDefinition.h"
#include "EmberMissionSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEmberCheckpointObjectiveRestoreTest,
    "ProjectEmber.Save.CheckpointObjectiveRestore",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEmberCheckpointObjectiveRestoreTest::RunTest(const FString& Parameters)
{
    UEmberMissionDefinition* Definition = NewObject<UEmberMissionDefinition>();
    Definition->Identifier = TEXT("HarborTest");
    FEmberObjectiveDefinition Objective;
    Objective.Identifier = TEXT("Insertion");
    Objective.Title = FText::FromString(TEXT("Insertion"));
    Definition->Objectives.Add(Objective);

    UEmberMissionSubsystem* Subsystem = NewObject<UEmberMissionSubsystem>();
    TestTrue(TEXT("Valid mission starts"), Subsystem->StartMission(Definition));
    TestTrue(TEXT("Objective completes"), Subsystem->SetObjectiveState(TEXT("Insertion"), EEmberObjectiveState::Completed));
    const FGuid CheckpointId = FGuid::NewGuid();
    const FEmberCheckpointSnapshot Snapshot = Subsystem->CaptureCheckpoint(CheckpointId);
    TestTrue(TEXT("Objective can change after capture"), Subsystem->SetObjectiveState(TEXT("Insertion"), EEmberObjectiveState::Failed));
    TestTrue(TEXT("Matching checkpoint schema restores"), Subsystem->RestoreCheckpoint(Snapshot));
    TestEqual(TEXT("Objective state restored"), Subsystem->GetObjectiveState(TEXT("Insertion")), EEmberObjectiveState::Completed);
    return true;
}

