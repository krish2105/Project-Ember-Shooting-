#include "Misc/AutomationTest.h"
#include "EmberTacticalStateComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEmberAIStateTerminalTest,
    "ProjectEmber.AI.DeadStateIsTerminal",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEmberAIStateTerminalTest::RunTest(const FString& Parameters)
{
    UEmberTacticalStateComponent* State = NewObject<UEmberTacticalStateComponent>();
    TestTrue(TEXT("AI can enter dead state"), State->SetState(EEmberAIState::Dead));
    TestFalse(TEXT("AI cannot leave dead state"), State->SetState(EEmberAIState::Patrol));
    TestEqual(TEXT("Dead remains terminal"), State->GetState(), EEmberAIState::Dead);
    return true;
}

