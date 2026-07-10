#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EmberTypes.h"
#include "EmberHUDState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEmberHUDStateUpdated);

UCLASS(BlueprintType)
class EMBERUI_API UEmberHUDState : public UObject
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) void SetVitals(float InHealth, float InArmor);
    UFUNCTION(BlueprintCallable) void SetAmmunition(int32 InMagazine, int32 InReserve);
    UPROPERTY(BlueprintReadOnly) float Health = 100.0f;
    UPROPERTY(BlueprintReadOnly) float Armor = 0.0f;
    UPROPERTY(BlueprintReadOnly) int32 MagazineAmmo = 0;
    UPROPERTY(BlueprintReadOnly) int32 ReserveAmmo = 0;
    UPROPERTY(BlueprintReadOnly) bool bMuzzleObstructed = false;
    UPROPERTY(BlueprintAssignable) FEmberHUDStateUpdated OnUpdated;
};

