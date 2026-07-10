#include "EmberDamageReceiverComponent.h"
#include "EmberArmorComponent.h"
#include "EmberHealthComponent.h"
#include "GameFramework/Actor.h"

FEmberDamageResult UEmberDamageReceiverComponent::ApplyDamageSpec(const FEmberDamageSpec& Spec)
{
    FEmberDamageResult Result;
    AActor* Owner = GetOwner();
    if (!Owner) return Result;

    const float ScaledDamage = FMath::Max(0.0f, Spec.BaseDamage * Spec.BodyPartModifier * Spec.DistanceModifier);
    float Remaining = ScaledDamage;
    if (UEmberArmorComponent* Armor = Owner->FindComponentByClass<UEmberArmorComponent>())
    {
        Remaining = Armor->AbsorbDamage(ScaledDamage, Spec.ArmorModifier, Result.AppliedToArmor);
        Result.bArmoredHit = Result.AppliedToArmor > 0.0f;
    }
    if (UEmberHealthComponent* Health = Owner->FindComponentByClass<UEmberHealthComponent>())
    {
        Result.AppliedToHealth = Health->ApplyHealthDamage(Remaining);
        Result.bKilled = Health->IsDead();
    }
    return Result;
}
