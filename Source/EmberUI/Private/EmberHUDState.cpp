#include "EmberHUDState.h"

void UEmberHUDState::SetVitals(float InHealth, float InArmor)
{
    Health = FMath::Max(0.0f, InHealth);
    Armor = FMath::Max(0.0f, InArmor);
    OnUpdated.Broadcast();
}

void UEmberHUDState::SetAmmunition(int32 InMagazine, int32 InReserve)
{
    MagazineAmmo = FMath::Max(0, InMagazine);
    ReserveAmmo = FMath::Max(0, InReserve);
    OnUpdated.Broadcast();
}

