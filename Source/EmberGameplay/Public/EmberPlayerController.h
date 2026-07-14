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
    UModularVehicleBaseComponent* GetVehicleSimulation() const;
    void ApplyVehicleLongitudinal(float Value);
    void ApplyVehicleSteering(float Value);
    void ApplyVehicleLookYaw(float Value);
    void ApplyVehicleLookPitch(float Value);
    void VehicleHandbrakePressed();
    void VehicleHandbrakeReleased();
    void VehicleInteractPressed();
    void BeginVehicleSmokeMotion();
    void CaptureVehicleSmoke();
    void CompleteVehicleSmoke();

    FTimerHandle GameplayInputActivationTimer;
    FTimerHandle VehicleSmokeStartTimer;
    FTimerHandle VehicleSmokeCaptureTimer;
    FTimerHandle VehicleSmokeExitTimer;
    FVector VehicleSmokeStartLocation = FVector::ZeroVector;
    double LastVehicleEnterTime = -1000.0;
    bool bVehicleHandbrake = false;
    bool bVehicleDriveInputConfirmed = false;
};
