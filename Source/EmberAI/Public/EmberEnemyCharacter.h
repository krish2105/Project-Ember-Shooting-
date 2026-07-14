#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EmberInterfaces.h"
#include "EmberEnemyCharacter.generated.h"

class UEmberArmorComponent;
class UEmberDamageReceiverComponent;
class UEmberHealthComponent;
class UEmberTacticalStateComponent;
class UTextRenderComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class UAudioComponent;
class USoundWaveProcedural;

UCLASS()
class EMBERAI_API AEmberEnemyCharacter : public ACharacter, public IEmberDamageable
{
    GENERATED_BODY()
public:
    AEmberEnemyCharacter();
    virtual void BeginPlay() override;
    virtual FEmberDamageResult ReceiveEmberDamage_Implementation(const FEmberDamageSpec& DamageSpec) override;
    UFUNCTION(BlueprintPure) bool IsAlive() const;
    UFUNCTION(BlueprintPure) UEmberHealthComponent* GetHealthComponent() const { return Health; }
    /** Executes one visible, authoritative enemy rifle shot. */
    bool FireAt(AActor* Target);
protected:
    UFUNCTION() void HandleDeath();
    void PlayHitReaction(const FEmberDamageSpec& DamageSpec);
    void ResetHitReaction();
    void ResetMuzzleFlash();
    void ResetShotTracer();
    void InitializeGunshotAudio();
    void PlayGunshotAudio();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberHealthComponent> Health;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberArmorComponent> Armor;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberDamageReceiverComponent> DamageReceiver;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberTacticalStateComponent> TacticalState;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UTextRenderComponent> HostileMarker;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat") TObjectPtr<UStaticMeshComponent> WeaponVisual;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat") TObjectPtr<UStaticMeshComponent> ShotTracerVisual;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat") TObjectPtr<UPointLightComponent> MuzzleFlashLight;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat") TObjectPtr<UAudioComponent> GunshotAudio;
    UPROPERTY(Transient) TObjectPtr<USoundWaveProcedural> GunshotWave;
    FTransform RestingMeshTransform;
    FTimerHandle HitReactionTimer;
    FTimerHandle MuzzleFlashTimer;
    FTimerHandle ShotTracerTimer;
    TArray<uint8> GunshotPCM;
};
