#include "Misc/AutomationTest.h"
#include "EmberWeaponDefinition.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEmberWeaponStableIdentityTest,
    "ProjectEmber.Core.WeaponStableIdentity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEmberWeaponStableIdentityTest::RunTest(const FString& Parameters)
{
    UEmberWeaponDefinition* Definition = NewObject<UEmberWeaponDefinition>();
    Definition->Identifier = TEXT("Weapon.StandardRifle");
    const FPrimaryAssetId Id = Definition->GetPrimaryAssetId();
    TestEqual(TEXT("Primary asset type is stable"), Id.PrimaryAssetType.ToString(), FString(TEXT("EmberWeapon")));
    TestEqual(TEXT("Primary asset name uses semantic identifier"), Id.PrimaryAssetName, FName(TEXT("Weapon.StandardRifle")));
    return true;
}
