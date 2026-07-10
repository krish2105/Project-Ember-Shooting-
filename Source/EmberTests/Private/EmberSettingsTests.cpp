#include "Misc/AutomationTest.h"
#include "EmberUserSettings.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEmberAccessibilityClampTest,
    "ProjectEmber.UI.AccessibilityClamps",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEmberAccessibilityClampTest::RunTest(const FString& Parameters)
{
    UEmberUserSettings* Settings = NewObject<UEmberUserSettings>();
    Settings->SetCameraShakeStrength(2.0f);
    Settings->SetSubtitleScale(5.0f);
    Settings->SetControllerDeadZone(-1.0f);
    TestEqual(TEXT("Camera shake is clamped"), Settings->GetCameraShakeStrength(), 1.0f);
    TestEqual(TEXT("Subtitle scale is clamped"), Settings->GetSubtitleScale(), 2.0f);
    TestEqual(TEXT("Dead zone is clamped"), Settings->GetControllerDeadZone(), 0.0f);
    return true;
}

