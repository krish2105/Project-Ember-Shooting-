#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundWaveProcedural.h"
#include "EmberVehicleSeatComponent.generated.h"

class AEmberCharacter;
class UAudioComponent;
class UCameraComponent;
class UPoseableMeshComponent;
class USpringArmComponent;

/** Continuously generated, dependency-free engine loop used by occupied cars. */
UCLASS()
class EMBERGAMEPLAY_API UEmberVehicleEngineSoundWave : public USoundWaveProcedural
{
    GENERATED_BODY()
public:
    UEmberVehicleEngineSoundWave(const FObjectInitializer& ObjectInitializer);
    virtual int32 OnGeneratePCMAudio(TArray<uint8>& OutAudio, int32 NumSamples) override;
    void SetEngineState(float InNormalizedRPM, float InLoad);

private:
    TAtomic<float> NormalizedRPM { 0.0f };
    TAtomic<float> EngineLoad { 0.0f };
    double Phase = 0.0;
    double SubPhase = 0.0;
};

/** Runtime bridge between an Ember operator and Epic's modular vehicle pawn. */
UCLASS(ClassGroup=(Ember), meta=(BlueprintSpawnableComponent))
class EMBERGAMEPLAY_API UEmberVehicleSeatComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UEmberVehicleSeatComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    bool SeatDriver(AEmberCharacter* InDriver);
    AEmberCharacter* ReleaseDriver(FVector& OutExitLocation, FRotator& OutExitRotation);
    void SetControlState(float InThrottle, float InSteering, float InBrake, bool bInHandbrake);
    void AddCameraInput(float YawDelta, float PitchDelta);

    UFUNCTION(BlueprintPure, Category="Ember|Vehicle")
    AEmberCharacter* GetDriverCharacter() const { return DriverCharacter.Get(); }
    UFUNCTION(BlueprintPure, Category="Ember|Vehicle")
    float GetSpeedKPH() const;
    UFUNCTION(BlueprintPure, Category="Ember|Vehicle")
    float GetForwardSpeedKPH() const;
    UFUNCTION(BlueprintPure, Category="Ember|Vehicle")
    float GetThrottleInput() const { return ThrottleInput; }
    UFUNCTION(BlueprintPure, Category="Ember|Vehicle")
    float GetSteeringInput() const { return SteeringInput; }
    UFUNCTION(BlueprintPure, Category="Ember|Vehicle")
    float GetBrakeInput() const { return BrakeInput; }
    UFUNCTION(BlueprintPure, Category="Ember|Vehicle")
    bool IsHandbrakeApplied() const { return bHandbrake; }

private:
    void BuildVisibleDriverPose(AEmberCharacter* InDriver);
    void BuildVehicleCamera();
    void DestroyVehicleCamera();
    void StartEngineAudio();
    FVector FindSafeExitLocation(AEmberCharacter* Driver) const;

    TWeakObjectPtr<AEmberCharacter> DriverCharacter;
    UPROPERTY(Transient) TObjectPtr<UPoseableMeshComponent> DriverPoseMesh;
    UPROPERTY(Transient) TObjectPtr<USpringArmComponent> VehicleCameraBoom;
    UPROPERTY(Transient) TObjectPtr<UCameraComponent> VehicleCamera;
    UPROPERTY(Transient) TObjectPtr<UAudioComponent> EngineAudio;
    UPROPERTY(Transient) TObjectPtr<UEmberVehicleEngineSoundWave> EngineWave;

    float ThrottleInput = 0.0f;
    float SteeringInput = 0.0f;
    float BrakeInput = 0.0f;
    bool bHandbrake = false;
    bool bMotionLogged = false;
    float CameraYawOffset = 0.0f;
    float CameraPitch = -8.0f;
};
