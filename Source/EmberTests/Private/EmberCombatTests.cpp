#include "Misc/AutomationTest.h"
#include "EmberBallisticsLibrary.h"
#include "EmberTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEmberDamageCalculationTest,
    "ProjectEmber.Combat.DamageCalculation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEmberDamageCalculationTest::RunTest(const FString& Parameters)
{
    FEmberDamageSpec Spec;
    Spec.BaseDamage = 40.0f;
    Spec.ArmorModifier = 0.5f;
    Spec.BodyPartModifier = 2.0f;
    Spec.DistanceModifier = 0.75f;
    TestEqual(TEXT("Damage modifiers are deterministic"), UEmberBallisticsLibrary::CalculateDamage(Spec), 30.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEmberFireCadenceTest,
    "ProjectEmber.Combat.FireCadence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEmberFireCadenceTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("600 RPM equals 0.1 seconds per shot"), UEmberBallisticsLibrary::SecondsPerShot(600.0f), 0.1f);
    TestTrue(TEXT("Invalid RPM cannot produce a usable cadence"), UEmberBallisticsLibrary::SecondsPerShot(0.0f) >= BIG_NUMBER);
    return true;
}
