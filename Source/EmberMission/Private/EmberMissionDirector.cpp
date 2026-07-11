#include "EmberMissionDirector.h"
#include "EmberHealthComponent.h"
#include "EmberMissionDefinition.h"
#include "EmberMissionSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

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

    TArray<AActor*> Enemies;
    UGameplayStatics::GetAllActorsWithTag(this, TEXT("EmberEnemy"), Enemies);
    RemainingEnemies = Enemies.Num();
    const FVector P = Player->GetActorLocation();

    if (P.X > -43000.0f) Complete(TEXT("Insertion"));
    if (P.X > -22000.0f) Complete(TEXT("FirstPatrol"));
    if (P.Y > 26000.0f) Complete(TEXT("StealthRoute"));
    if (P.X > -5000.0f) Complete(TEXT("Warehouse"));
    if (P.X > 9000.0f)
    {
        Complete(TEXT("MidpointCheckpoint"));
        if (!bCheckpointReached)
        {
            bCheckpointReached = true;
            LastCheckpointLocation = P;
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
        Health->RestoreToFull();
        Player->SetActorLocation(LastCheckpointLocation, false, nullptr, ETeleportType::TeleportPhysics);
    }
}
