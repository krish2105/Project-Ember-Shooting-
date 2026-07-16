#include "EmberInteractionComponent.h"
#include "EmberInterfaces.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

bool UEmberInteractionComponent::TryInteract(const FVector& ViewOrigin, const FVector& ViewDirection)
{
    UWorld* World = GetWorld();
    if (!World) return false;
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(EmberInteraction), false, GetOwner());
    if (!World->LineTraceSingleByChannel(Hit, ViewOrigin, ViewOrigin + ViewDirection.GetSafeNormal() * InteractionRange, ECC_Visibility, Params)) return false;
    AActor* Target = Hit.GetActor();
    if (!Target || !Target->GetClass()->ImplementsInterface(UEmberInteractable::StaticClass())) return false;
    if (!IEmberInteractable::Execute_CanInteract(Target, GetOwner())) return false;
    IEmberInteractable::Execute_Interact(Target, GetOwner());
    return true;
}

APawn* UEmberInteractionComponent::FindDriveableVehicle(const FVector& ViewOrigin, const FVector& ViewDirection) const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;
    const FVector End = ViewOrigin + ViewDirection.GetSafeNormal() * InteractionRange;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(EmberVehicleInteraction), false, GetOwner());
    TArray<FHitResult> Hits;
    World->SweepMultiByChannel(Hits, ViewOrigin, End, FQuat::Identity, ECC_Visibility,
        FCollisionShape::MakeSphere(72.0f), Params);
    for (const FHitResult& Hit : Hits)
    {
        APawn* Candidate = Cast<APawn>(Hit.GetActor());
        if (Candidate && Candidate->ActorHasTag(TEXT("EmberDriveableVehicle"))
            && (!Candidate->GetController() || !Candidate->GetController()->IsPlayerController()))
        {
            return Candidate;
        }
    }

    // Vehicle entry should be proximity based rather than pixel-perfect. A
    // player standing beside a door can enter even when the over-shoulder
    // camera is looking slightly past the bodywork.
    const FVector OwnerLocation = GetOwner() ? GetOwner()->GetActorLocation() : ViewOrigin;
    TArray<FOverlapResult> Nearby;
    if (World->OverlapMultiByObjectType(Nearby, OwnerLocation, FQuat::Identity,
        FCollisionObjectQueryParams::AllDynamicObjects,
        FCollisionShape::MakeSphere(InteractionRange + 100.0f), Params))
    {
        APawn* Nearest = nullptr;
        float NearestDistanceSquared = TNumericLimits<float>::Max();
        for (const FOverlapResult& Result : Nearby)
        {
            APawn* Candidate = Cast<APawn>(Result.GetActor());
            if (!Candidate || !Candidate->ActorHasTag(TEXT("EmberDriveableVehicle"))
                || (Candidate->GetController() && Candidate->GetController()->IsPlayerController())) continue;
            const float DistanceSquared = FVector::DistSquared(OwnerLocation, Candidate->GetActorLocation());
            if (DistanceSquared < NearestDistanceSquared)
            {
                Nearest = Candidate;
                NearestDistanceSquared = DistanceSquared;
            }
        }
        return Nearest;
    }
    return nullptr;
}
