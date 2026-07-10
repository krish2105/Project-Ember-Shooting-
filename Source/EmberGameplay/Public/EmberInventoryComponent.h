#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EmberInventoryComponent.generated.h"

class UEmberWeaponDefinition;

UCLASS(ClassGroup=(Ember), meta=(BlueprintSpawnableComponent))
class EMBERGAMEPLAY_API UEmberInventoryComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) bool AddWeapon(TSoftObjectPtr<UEmberWeaponDefinition> Definition);
    UFUNCTION(BlueprintCallable) bool SelectWeapon(int32 Index);
    UFUNCTION(BlueprintPure) int32 GetSelectedIndex() const { return SelectedIndex; }
    UFUNCTION(BlueprintPure) const TArray<TSoftObjectPtr<UEmberWeaponDefinition>>& GetWeapons() const { return Weapons; }
protected:
    UPROPERTY(EditAnywhere, Category="Inventory") TArray<TSoftObjectPtr<UEmberWeaponDefinition>> Weapons;
    UPROPERTY(VisibleInstanceOnly, Category="Inventory") int32 SelectedIndex = INDEX_NONE;
};

