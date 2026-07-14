#include "Misc/AutomationTest.h"
#include "EmberBallisticsLibrary.h"
#include "EmberTypes.h"
#include "EmberWeaponComponent.h"
#include "EmberWeaponDefinition.h"

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
    Spec.ShotDirection = FVector(1.0f, 0.0f, 0.0f);
    Spec.ImpactPoint = FVector(100.0f, 20.0f, 50.0f);
    TestTrue(TEXT("Damage contract carries a normalized hit-reaction direction"),
        FVector(Spec.ShotDirection).Equals(FVector::ForwardVector));
    TestTrue(TEXT("Damage contract carries the impact point"),
        FVector(Spec.ImpactPoint).Equals(FVector(100.0f, 20.0f, 50.0f)));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEmberWeaponStateRestoreTest,
    "ProjectEmber.Combat.WeaponStateRestore",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEmberWeaponStateRestoreTest::RunTest(const FString& Parameters)
{
    UEmberWeaponDefinition* Definition = NewObject<UEmberWeaponDefinition>();
    Definition->MagazineCapacity = 30;
    Definition->RoundsPerMinute = 600.0f;
    Definition->SupportedFireModes = { EEmberFireMode::FullyAutomatic };
    UEmberWeaponComponent* Weapon = NewObject<UEmberWeaponComponent>();
    TestTrue(TEXT("A saved weapon state initializes"),
        Weapon->InitializeWeaponState(Definition, 11, 73));
    TestEqual(TEXT("Saved magazine ammunition is restored"), Weapon->GetMagazineAmmo(), 11);
    TestEqual(TEXT("Saved reserve ammunition is restored"), Weapon->GetReserveAmmo(), 73);
    TestTrue(TEXT("Restored automatic weapon keeps its fire mode"), Weapon->IsAutomatic());
    TestTrue(TEXT("Aim spread is tighter than hip spread"),
        Weapon->GetSpreadDegrees(true) < Weapon->GetSpreadDegrees(false));
    return true;
}
