#include "EmberInteractionComponent.h"
#include "EmberInterfaces.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

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
