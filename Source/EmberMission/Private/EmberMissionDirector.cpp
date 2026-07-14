#include "EmberMissionDirector.h"
#include "EmberHealthComponent.h"
#include "EmberArmorComponent.h"
#include "EmberMissionDefinition.h"
#include "EmberMissionSubsystem.h"
#include "EmberMissionSaveGame.h"
#include "EmberLog.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

const FString AEmberMissionDirector::CheckpointSlot(TEXT("ProjectEmberCheckpoint"));

AEmberMissionDirector::AEmberMissionDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
    Tags.Add(TEXT("EmberMissionDirector"));
    static ConstructorHelpers::FObjectFinder<UEmberMissionDefinition> MissionAsset(
        TEXT("/Game/Ember/Missions/DA_Mission_Harbor.DA_Mission_Harbor"));
    if (MissionAsset.Succeeded()) MissionDefinition = MissionAsset.Object;
}

void AEmberMissionDirector::BeginPlay()
{
    Super::BeginPlay();
    if (UEmberMissionSubsystem* Mission = GetWorld()->GetSubsystem<UEmberMissionSubsystem>())
    {
        Mission->StartMission(MissionDefinition);
    }
    CacheEncounterActors();
    GetWorldTimerManager().SetTimerForNextTick(this, &AEmberMissionDirector::RestoreSavedCheckpoint);
}

void AEmberMissionDirector::CacheEncounterActors()
{
    TArray<AActor*> Enemies;
    UGameplayStatics::GetAllActorsWithTag(this, TEXT("EmberEnemy"), Enemies);
    TrackedEnemies.Reset(Enemies.Num());
    for (AActor* Enemy : Enemies) TrackedEnemies.Add(Enemy);

    TArray<AActor*> FirstPatrol;
    UGameplayStatics::GetAllActorsWithTag(this, TEXT("EmberPatrolOne"), FirstPatrol);
    TrackedFirstPatrol.Reset(FirstPatrol.Num());
    for (AActor* Enemy : FirstPatrol) TrackedFirstPatrol.Add(Enemy);
    UE_LOG(LogEmberMission, Log, TEXT("Cached %d mission enemies (%d in first patrol)"),
        TrackedEnemies.Num(), TrackedFirstPatrol.Num());
}

int32 AEmberMissionDirector::CountLivingActors(const TArray<TWeakObjectPtr<AActor>>& Actors) const
{
    int32 Count = 0;
    for (const TWeakObjectPtr<AActor>& Entry : Actors)
    {
        const AActor* Actor = Entry.Get();
        if (!Actor) continue;
        const UEmberHealthComponent* Health = Actor->FindComponentByClass<UEmberHealthComponent>();
        if (!Health || !Health->IsDead()) ++Count;
    }
    return Count;
}

void AEmberMissionDirector::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateAccumulator += DeltaSeconds;
    if (UpdateAccumulator >= 0.25f)
    {
        UpdateAccumulator = 0.0f;
        UpdateMission();
    }
}

void AEmberMissionDirector::Complete(FName ObjectiveId)
{
    if (UEmberMissionSubsystem* Mission = GetWorld()->GetSubsystem<UEmberMissionSubsystem>())
    {
        if (Mission->GetObjectiveState(ObjectiveId) == EEmberObjectiveState::Active)
            Mission->SetObjectiveState(ObjectiveId, EEmberObjectiveState::Completed);
    }
}

void AEmberMissionDirector::UpdateMission()
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player) return;

    RemainingEnemies = CountLivingActors(TrackedEnemies);
    const int32 FirstPatrolRemaining = CountLivingActors(TrackedFirstPatrol);
    const FVector P = Player->GetActorLocation();

    if (P.X > -43000.0f) Complete(TEXT("Insertion"));
    if (FirstPatrolRemaining == 0) Complete(TEXT("FirstPatrol"));
    if (P.Y > 26000.0f) Complete(TEXT("StealthRoute"));
    if (P.X > -5000.0f) Complete(TEXT("Warehouse"));
    if (P.X > 9000.0f)
    {
        Complete(TEXT("MidpointCheckpoint"));
        if (!bCheckpointReached)
        {
            bCheckpointReached = true;
            LastCheckpointLocation = P;
            SaveCheckpoint();
        }
    }
    if (P.X > 18000.0f) Complete(TEXT("CraneOverlook"));
    if (P.X > 28000.0f && RemainingEnemies <= 5) Complete(TEXT("ContainerArena"));
    if (FVector::DistSquared2D(P, FVector(36000.0f, 36000.0f, P.Z)) < FMath::Square(3000.0f))
        Complete(TEXT("SecondaryManifest"));
    if (RemainingEnemies <= 2 && FVector::DistSquared2D(P, FVector(43000.0f, -38000.0f, P.Z)) < FMath::Square(3500.0f))
    {
        Complete(TEXT("Extraction"));
        Complete(TEXT("Results"));
        bMissionComplete = true;
    }

    if (UEmberHealthComponent* Health = Player->FindComponentByClass<UEmberHealthComponent>(); Health && Health->IsDead())
    {
        if (UPawnMovementComponent* Movement = Player->GetMovementComponent()) Movement->StopMovementImmediately();
        Health->RestoreToFull();
        if (UEmberArmorComponent* Armor = Player->FindComponentByClass<UEmberArmorComponent>())
            Armor->RestoreArmor(100.0f);
        Player->SetActorLocation(LastCheckpointLocation, false, nullptr, ETeleportType::TeleportPhysics);
        UE_LOG(LogEmberMission, Log, TEXT("Player recovered at checkpoint with restored health and armor"));
    }
}

void AEmberMissionDirector::SaveCheckpoint()
{
    UEmberMissionSubsystem* Mission = GetWorld()->GetSubsystem<UEmberMissionSubsystem>();
    if (!Mission) return;
    UEmberMissionSaveGame* Save = Cast<UEmberMissionSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UEmberMissionSaveGame::StaticClass()));
    if (!Save) return;
    Save->Snapshot = Mission->CaptureCheckpoint(FGuid::NewGuid());
    const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, CheckpointSlot, 0);
    UE_LOG(LogEmberMission, Log, TEXT("Checkpoint save %s"), bSaved ? TEXT("succeeded") : TEXT("failed"));
}

void AEmberMissionDirector::RestoreSavedCheckpoint()
{
    if (!UGameplayStatics::DoesSaveGameExist(CheckpointSlot, 0)) return;
    UEmberMissionSaveGame* Save = Cast<UEmberMissionSaveGame>(
        UGameplayStatics::LoadGameFromSlot(CheckpointSlot, 0));
    UEmberMissionSubsystem* Mission = GetWorld()->GetSubsystem<UEmberMissionSubsystem>();
    if (Save && Mission && Mission->RestoreCheckpoint(Save->Snapshot))
    {
        LastCheckpointLocation = Save->Snapshot.PlayerTransform.GetLocation();
        bCheckpointReached = true;
        UE_LOG(LogEmberMission, Log, TEXT("Checkpoint restored"));
    }
}
