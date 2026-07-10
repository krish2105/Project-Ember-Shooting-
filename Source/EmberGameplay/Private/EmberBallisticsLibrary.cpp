#include "EmberBallisticsLibrary.h"

float UEmberBallisticsLibrary::CalculateDamage(const FEmberDamageSpec& Spec)
{
    return FMath::Max(0.0f, Spec.BaseDamage * Spec.ArmorModifier * Spec.BodyPartModifier * Spec.DistanceModifier);
}

float UEmberBallisticsLibrary::SecondsPerShot(float RoundsPerMinute)
{
    return RoundsPerMinute > KINDA_SMALL_NUMBER ? 60.0f / RoundsPerMinute : BIG_NUMBER;
}

