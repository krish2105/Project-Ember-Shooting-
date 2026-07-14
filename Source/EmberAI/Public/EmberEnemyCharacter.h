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

UCLASS()
class EMBERAI_API AEmberEnemyCharacter : public ACharacter, public IEmberDamageable
{
    GENERATED_BODY()
public:
    AEmberEnemyCharacter();
    virtual void BeginPlay() override;
    virtual FEmberDamageResult ReceiveEmberDamage_Implementation(const FEmberDamageSpec& DamageSpec) override;
    UFUNCTION(BlueprintPure) bool IsAlive() const;
protected:
    UFUNCTION() void HandleDeath();
    void PlayHitReaction(const FEmberDamageSpec& DamageSpec);
    void ResetHitReaction();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberHealthComponent> Health;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberArmorComponent> Armor;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberDamageReceiverComponent> DamageReceiver;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UEmberTacticalStateComponent> TacticalState;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UTextRenderComponent> HostileMarker;
    FTransform RestingMeshTransform;
    FTimerHandle HitReactionTimer;
};
