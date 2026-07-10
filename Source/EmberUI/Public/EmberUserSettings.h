#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "EmberUserSettings.generated.h"

UCLASS(Config=GameUserSettings)
class EMBERUI_API UEmberUserSettings : public UGameUserSettings
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) void SetCameraShakeStrength(float Value);
    UFUNCTION(BlueprintCallable) void SetSubtitleScale(float Value);
    UFUNCTION(BlueprintCallable) void SetControllerDeadZone(float Value);
    UFUNCTION(BlueprintPure) float GetCameraShakeStrength() const { return CameraShakeStrength; }
    UFUNCTION(BlueprintPure) float GetSubtitleScale() const { return SubtitleScale; }
    UFUNCTION(BlueprintPure) float GetControllerDeadZone() const { return ControllerDeadZone; }
    virtual void SetToDefaults() override;
protected:
    UPROPERTY(Config) float CameraShakeStrength = 1.0f;
    UPROPERTY(Config) float SubtitleScale = 1.0f;
    UPROPERTY(Config) float ControllerDeadZone = 0.15f;
};
