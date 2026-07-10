#include "EmberUserSettings.h"

void UEmberUserSettings::SetCameraShakeStrength(float Value) { CameraShakeStrength = FMath::Clamp(Value, 0.0f, 1.0f); }
void UEmberUserSettings::SetSubtitleScale(float Value) { SubtitleScale = FMath::Clamp(Value, 0.75f, 2.0f); }
void UEmberUserSettings::SetControllerDeadZone(float Value) { ControllerDeadZone = FMath::Clamp(Value, 0.0f, 0.5f); }

void UEmberUserSettings::SetToDefaults()
{
    Super::SetToDefaults();
    CameraShakeStrength = 1.0f;
    SubtitleScale = 1.0f;
    ControllerDeadZone = 0.15f;
}

