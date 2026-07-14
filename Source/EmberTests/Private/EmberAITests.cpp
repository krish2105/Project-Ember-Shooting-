#include "Misc/AutomationTest.h"
#include "EmberTacticalStateComponent.h"
#include "EmberEnemyCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkeletalMeshComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEmberAIStateTerminalTest,
    "ProjectEmber.AI.DeadStateIsTerminal",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEmberAIStateTerminalTest::RunTest(const FString& Parameters)
{
    UEmberTacticalStateComponent* State = NewObject<UEmberTacticalStateComponent>();
    TestTrue(TEXT("AI can enter dead state"), State->SetState(EEmberAIState::Dead));
    TestFalse(TEXT("AI cannot leave dead state"), State->SetState(EEmberAIState::Patrol));
    TestEqual(TEXT("Dead remains terminal"), State->GetState(), EEmberAIState::Dead);
    AEmberEnemyCharacter* Enemy = GetMutableDefault<AEmberEnemyCharacter>();
    TestTrue(TEXT("Enemy actor yaw follows the tactical controller"), Enemy->bUseControllerRotationYaw);
    TestTrue(TEXT("Manny keeps the standard actor-forward mesh basis"),
        FMath::IsNearlyEqual(Enemy->GetMesh()->GetRelativeRotation().Yaw, -90.0f, 0.1f));
    const UStaticMeshComponent* Weapon = Cast<UStaticMeshComponent>(
        Enemy->GetDefaultSubobjectByName(TEXT("EnemyWeaponVisual")));
    TestNotNull(TEXT("Enemy visibly carries a rifle"), Weapon);
    if (Weapon)
    {
        TestEqual(TEXT("Enemy rifle follows the two-handed grip socket"),
            Weapon->GetAttachSocketName(), FName(TEXT("HandGrip_R")));
    }
    TestNotNull(TEXT("Enemy fire has a bounded muzzle flash"),
        Enemy->GetDefaultSubobjectByName(TEXT("EnemyMuzzleFlash")));
    TestNotNull(TEXT("Enemy fire has a reusable tracer"),
        Enemy->GetDefaultSubobjectByName(TEXT("EnemyShotTracer")));
    TestNotNull(TEXT("Enemy fire has a reusable spatial gunshot source"),
        Enemy->GetDefaultSubobjectByName(TEXT("EnemyGunshotAudio")));
    return true;
}
