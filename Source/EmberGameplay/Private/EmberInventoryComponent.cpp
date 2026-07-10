#include "EmberInventoryComponent.h"
#include "EmberWeaponDefinition.h"

bool UEmberInventoryComponent::AddWeapon(TSoftObjectPtr<UEmberWeaponDefinition> Definition)
{
    if (Definition.IsNull() || Weapons.Contains(Definition)) return false;
    Weapons.Add(Definition);
    if (SelectedIndex == INDEX_NONE) SelectedIndex = 0;
    return true;
}

bool UEmberInventoryComponent::SelectWeapon(int32 Index)
{
    if (!Weapons.IsValidIndex(Index)) return false;
    SelectedIndex = Index;
    return true;
}

