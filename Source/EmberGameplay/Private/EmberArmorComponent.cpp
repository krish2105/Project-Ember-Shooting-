#include "EmberArmorComponent.h"

UEmberArmorComponent::UEmberArmorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

float UEmberArmorComponent::AbsorbDamage(float IncomingDamage, float ArmorModifier, float& OutAbsorbed)
{
    const float EffectiveDamage = FMath::Max(0.0f, IncomingDamage * ArmorModifier);
    OutAbsorbed = FMath::Min(CurrentArmor, EffectiveDamage * ProtectionRatio);
    CurrentArmor -= OutAbsorbed;
    return FMath::Max(0.0f, EffectiveDamage - OutAbsorbed);
}

void UEmberArmorComponent::RestoreArmor(float Value)
{
    CurrentArmor = FMath::Clamp(Value, 0.0f, MaxArmor);
}
