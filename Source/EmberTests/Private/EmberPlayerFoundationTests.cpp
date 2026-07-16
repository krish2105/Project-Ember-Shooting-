#include "Misc/AutomationTest.h"
#include "EmberCharacter.h"
#include "EmberGameMode.h"
#include "EmberPlayerController.h"
#include "EmberVehicleSeatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/InputSettings.h"
#include "InputCoreTypes.h"
#include "Components/ActorComponent.h"
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
        TestEqual(TEXT("Human-weight gravity is enabled"), Movement->GravityScale, 1.35f);
        TestEqual(TEXT("Jump impulse remains compact"), Movement->JumpZVelocity, 520.0f);
        TestEqual(TEXT("Air control remains tactical"), Movement->AirControl, 0.28f);
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
    auto HasAxisKey = [InputSettings](const FName AxisName, const FKey ExpectedKey)
    {
        TArray<FInputAxisKeyMapping> Mappings;
        InputSettings->GetAxisMappingByName(AxisName, Mappings);
        return Mappings.ContainsByPredicate([ExpectedKey](const FInputAxisKeyMapping& Mapping)
        {
            return Mapping.Key == ExpectedKey;
        });
    };
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
    TestTrue(TEXT("Horizontal mouse look is mapped"), HasAxisKey(TEXT("Turn"), EKeys::MouseX));
    TestTrue(TEXT("Vertical mouse look is mapped"), HasAxisKey(TEXT("LookUp"), EKeys::MouseY));
    TestTrue(TEXT("Keyboard reload is mapped"), HasActionKey(TEXT("Reload"), EKeys::R));
    TestTrue(TEXT("Keyboard interact is mapped"), HasActionKey(TEXT("Interact"), EKeys::E));
    TestTrue(TEXT("Keyboard melee is mapped"), HasActionKey(TEXT("Melee"), EKeys::V));
    TestTrue(TEXT("Tactical overlay is mapped"), HasActionKey(TEXT("TacticalOverlay"), EKeys::Tab));
    TestTrue(TEXT("Controls overlay is mapped"), HasActionKey(TEXT("ControlsOverlay"), EKeys::H));
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
    TestFalse(TEXT("Hold-to-aim is the default input contract"), MutableCharacter->IsToggleAimEnabled());
    MutableCharacter->SetToggleAimEnabled(true);
    TestTrue(TEXT("Toggle aim remains available as an accessibility option"), MutableCharacter->IsToggleAimEnabled());
    MutableCharacter->SetToggleAimEnabled(false);

    const UStaticMeshComponent* WeaponVisual = Cast<UStaticMeshComponent>(
        MutableCharacter->GetDefaultSubobjectByName(TEXT("WeaponBodyVisual")));
    TestNotNull(TEXT("Modeled weapon visual exists"), WeaponVisual);
    if (WeaponVisual)
    {
        TestTrue(TEXT("Weapon follows the skeletal character mesh"),
            WeaponVisual->GetAttachParent() == MutableCharacter->GetMesh());
        TestEqual(TEXT("Weapon uses the Shooter Variant grip socket"),
            WeaponVisual->GetAttachSocketName(), FName(TEXT("HandGrip_R")));
        TestTrue(TEXT("Rifle snaps to the authored grip socket basis"),
            WeaponVisual->GetRelativeRotation().IsNearlyZero(0.1f));
    }
    const UStaticMeshComponent* TracerVisual = Cast<UStaticMeshComponent>(
        MutableCharacter->GetDefaultSubobjectByName(TEXT("WeaponBarrelVisual")));
    TestNotNull(TEXT("Transient tracer helper exists"), TracerVisual);
    if (TracerVisual)
    {
        TestFalse(TEXT("Tracer helper is not rendered as a fake gun"), TracerVisual->IsVisible());
    }
    const UStaticMeshComponent* PooledTracer = Cast<UStaticMeshComponent>(
        MutableCharacter->GetDefaultSubobjectByName(TEXT("ShotTracerVisual")));
    TestNotNull(TEXT("A reusable bounded tracer component exists"), PooledTracer);
    if (PooledTracer) TestFalse(TEXT("Reusable tracer begins hidden"), PooledTracer->IsVisible());
    TestNotNull(TEXT("A reusable bounded impact light exists"),
        MutableCharacter->GetDefaultSubobjectByName(TEXT("ImpactFeedbackLight")));
    TestNotNull(TEXT("A reusable gunshot audio component exists"),
        MutableCharacter->GetDefaultSubobjectByName(TEXT("GunshotAudio")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEmberVehicleAccessContractTest,
    "ProjectEmber.Vehicle.AccessAndSimulationContract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEmberVehicleAccessContractTest::RunTest(const FString& Parameters)
{
    const AEmberPlayerController* Controller = GetDefault<AEmberPlayerController>();
    TestNotNull(TEXT("Player controller exposes vehicle entry"),
        Controller->FindFunction(TEXT("EnterVehicle")));
    TestNotNull(TEXT("Player controller exposes vehicle exit"),
        Controller->FindFunction(TEXT("ExitVehicle")));
    TestNotNull(TEXT("Vehicle seat component exposes a visible driver contract"),
        UEmberVehicleSeatComponent::StaticClass()->FindFunctionByName(TEXT("GetDriverCharacter")));

    UClass* VehicleClass = LoadClass<APawn>(nullptr,
        TEXT("/ChaosModularVehicleExamples/Vehicles/ModularVehicle/SportsCar/"
             "BP_ModularVehicleSimplifiedSkeletalSetup.BP_ModularVehicleSimplifiedSkeletalSetup_C"));
    TestNotNull(TEXT("Licensed modular sports-car pawn is loadable"), VehicleClass);
    if (VehicleClass)
    {
        const APawn* VehicleCDO = Cast<APawn>(VehicleClass->GetDefaultObject());
        TestNotNull(TEXT("Sports-car class has a pawn default object"), VehicleCDO);
        bool bHasVehicleSimulation = false;
        if (VehicleCDO)
        {
            TArray<UActorComponent*> Components;
            VehicleCDO->GetComponents(Components);
            for (const UActorComponent* Component : Components)
            {
                if (!Component) continue;
                const FString ClassName = Component->GetClass()->GetName();
                bHasVehicleSimulation |= ClassName.Contains(TEXT("ModularVehicle"));
            }
        }
        TestTrue(TEXT("Sports car owns a Chaos modular vehicle simulation component"), bHasVehicleSimulation);
    }

    const UInputSettings* InputSettings = GetDefault<UInputSettings>();
    TArray<FInputActionKeyMapping> JumpMappings;
    InputSettings->GetActionMappingByName(TEXT("Jump"), JumpMappings);
    TestTrue(TEXT("Space is available as the vehicle handbrake while driving"),
        JumpMappings.ContainsByPredicate([](const FInputActionKeyMapping& Mapping)
        {
            return Mapping.Key == EKeys::SpaceBar;
        }));
    return true;
}
