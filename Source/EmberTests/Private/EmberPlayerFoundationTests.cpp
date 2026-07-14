#include "Misc/AutomationTest.h"
#include "EmberCharacter.h"
#include "EmberGameMode.h"
#include "EmberPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/InputSettings.h"
#include "InputCoreTypes.h"
#include "Components/StaticMeshComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEmberPlayerWalkingFoundationTest,
    "ProjectEmber.Player.WalkingFoundation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEmberPlayerWalkingFoundationTest::RunTest(const FString& Parameters)
{
    const AEmberCharacter* Character = GetDefault<AEmberCharacter>();
    const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    TestNotNull(TEXT("Character owns a movement component"), Movement);
    if (Movement)
    {
        TestEqual(TEXT("Normal gravity is enabled"), Movement->GravityScale, 1.0f);
        TestFalse(TEXT("Player is not constrained to a fake ground plane"), Movement->bConstrainToPlane);
        TestTrue(TEXT("Walking remains the default land movement mode"),
            Movement->DefaultLandMovementMode == MOVE_Walking);
    }

    const AEmberGameMode* GameMode = GetDefault<AEmberGameMode>();
    TestTrue(TEXT("Game mode uses the packaged-focus player controller"),
        GameMode->PlayerControllerClass == AEmberPlayerController::StaticClass());
    TestTrue(TEXT("All gameplay cameras remain on the third-person pawn"),
        GameMode->DefaultPawnClass == AEmberCharacter::StaticClass());

    const UInputSettings* InputSettings = GetDefault<UInputSettings>();
    auto HasActionKey = [InputSettings](const FName ActionName, const FKey ExpectedKey)
    {
        TArray<FInputActionKeyMapping> Mappings;
        InputSettings->GetActionMappingByName(ActionName, Mappings);
        return Mappings.ContainsByPredicate([ExpectedKey](const FInputActionKeyMapping& Mapping)
        {
            return Mapping.Key == ExpectedKey;
        });
    };
    TestTrue(TEXT("Mouse fire is mapped"), HasActionKey(TEXT("Fire"), EKeys::LeftMouseButton));
    TestTrue(TEXT("Mouse aim is mapped"), HasActionKey(TEXT("Aim"), EKeys::RightMouseButton));
    TestTrue(TEXT("Keyboard reload is mapped"), HasActionKey(TEXT("Reload"), EKeys::R));
    TestTrue(TEXT("Controller fire is mapped"), HasActionKey(TEXT("Fire"), EKeys::Gamepad_RightTrigger));
    TestTrue(TEXT("Controller aim is mapped"), HasActionKey(TEXT("Aim"), EKeys::Gamepad_LeftTrigger));

    AEmberCharacter* MutableCharacter = GetMutableDefault<AEmberCharacter>();
    MutableCharacter->SetAiming(true);
    TestTrue(TEXT("Aim state can be entered"), MutableCharacter->IsAiming());
    TestTrue(TEXT("Aim state turns the pawn with the controller"), MutableCharacter->bUseControllerRotationYaw);
    TestFalse(TEXT("Aim state disables orient-to-movement"),
        MutableCharacter->GetCharacterMovement()->bOrientRotationToMovement);
    MutableCharacter->SetAiming(false);
    TestFalse(TEXT("Aim state can be exited"), MutableCharacter->IsAiming());

    TestNotNull(TEXT("Weapon body visual exists"),
        Cast<UStaticMeshComponent>(MutableCharacter->GetDefaultSubobjectByName(TEXT("WeaponBodyVisual"))));
    TestNotNull(TEXT("Weapon barrel visual exists"),
        Cast<UStaticMeshComponent>(MutableCharacter->GetDefaultSubobjectByName(TEXT("WeaponBarrelVisual"))));
    TestNotNull(TEXT("Weapon stock visual exists"),
        Cast<UStaticMeshComponent>(MutableCharacter->GetDefaultSubobjectByName(TEXT("WeaponStockVisual"))));
    TestNotNull(TEXT("Weapon magazine visual exists"),
        Cast<UStaticMeshComponent>(MutableCharacter->GetDefaultSubobjectByName(TEXT("WeaponMagazineVisual"))));
    return true;
}
