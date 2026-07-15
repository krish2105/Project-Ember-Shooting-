#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EmberPlayerController.generated.h"

class AEmberCharacter;
class UEmberVehicleSeatComponent;
class UModularVehicleBaseComponent;

UCLASS()
class EMBERGAMEPLAY_API AEmberPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void SetupInputComponent() override;

    UFUNCTION(BlueprintCallable, Category="Ember|Vehicle")
    bool EnterVehicle(APawn* VehiclePawn, AEmberCharacter* DriverCharacter);
    UFUNCTION(BlueprintCallable, Category="Ember|Vehicle")
    void ExitVehicle();
    UFUNCTION(BlueprintPure, Category="Ember|Vehicle")
    bool IsDrivingVehicle() const;
    UFUNCTION(BlueprintPure, Category="Ember|Vehicle")
    UEmberVehicleSeatComponent* GetVehicleSeatComponent() const;

    /** Development-only native Metal smoke route; never invoked in normal play. */
    UFUNCTION(Exec)
    void EmberVehicleSmoke();

private:
    void ArmGameplayInput();
    void ActivateGameplayInput();
    void FocusGameplayViewport();
    void UpdateVehicleBridgeInputConsumption();
    void ResetVehicleForPlayerControl(APawn* VehiclePawn);
    UModularVehicleBaseComponent* GetVehicleSimulation() const;
    void ApplyVehicleLongitudinal(float Value);
    void ApplyVehicleSteering(float Value);
    void ApplyVehicleLookYaw(float Value);
    void ApplyVehicleLookPitch(float Value);
    void VehicleHandbrakePressed();
    void VehicleHandbrakeReleased();
    void VehicleInteractPressed();
    void ValidateVehicleSmokeIdle();
    void BeginVehicleSmokeMotion();
    void ReverseVehicleSmokeSteering();
    void NeutralizeVehicleSmokeSteering();
    void CaptureVehicleSmoke();
    void CompleteVehicleSmoke();

    FTimerHandle GameplayInputActivationTimer;
    FTimerHandle VehicleSmokeIdleTimer;
    FTimerHandle VehicleSmokeStartTimer;
    FTimerHandle VehicleSmokeReverseSteeringTimer;
    FTimerHandle VehicleSmokeNeutralSteeringTimer;
    FTimerHandle VehicleSmokeCaptureTimer;
    FTimerHandle VehicleSmokeExitTimer;
    FVector VehicleSmokeStartLocation = FVector::ZeroVector;
    float VehicleSmokeStartYaw = 0.0f;
    float VehicleSmokeRightTurnYaw = 0.0f;
    float VehicleSmokeLeftTurnYaw = 0.0f;
    double LastVehicleEnterTime = -1000.0;
    bool bVehicleHandbrake = false;
    bool bVehicleDriveInputConfirmed = false;
    bool bVehicleSteeringInputConfirmed = false;
};
