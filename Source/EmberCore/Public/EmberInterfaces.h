#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EmberTypes.h"
#include "EmberInterfaces.generated.h"

UINTERFACE(BlueprintType)
class EMBERCORE_API UEmberDamageable : public UInterface { GENERATED_BODY() };

class EMBERCORE_API IEmberDamageable
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ember|Damage")
    FEmberDamageResult ReceiveEmberDamage(const FEmberDamageSpec& DamageSpec);
};

UINTERFACE(BlueprintType)
class EMBERCORE_API UEmberInteractable : public UInterface { GENERATED_BODY() };

class EMBERCORE_API IEmberInteractable
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ember|Interaction")
    bool CanInteract(AActor* Interactor) const;
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ember|Interaction")
    void Interact(AActor* Interactor);
};

UINTERFACE(BlueprintType)
class EMBERCORE_API UEmberSaveParticipant : public UInterface { GENERATED_BODY() };

class EMBERCORE_API IEmberSaveParticipant
{
    GENERATED_BODY()
public:
    virtual FGuid GetStableSaveId() const = 0;
    virtual void CaptureSaveRecord(FEmberSaveParticipantRecord& OutRecord) const = 0;
    virtual bool RestoreSaveRecord(const FEmberSaveParticipantRecord& Record) = 0;
};

UINTERFACE(BlueprintType)
class EMBERCORE_API UEmberAudioRouter : public UInterface { GENERATED_BODY() };

class EMBERCORE_API IEmberAudioRouter
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ember|Audio")
    void RouteAudioEvent(FGameplayTag EventTag, const FVector& WorldLocation, float Intensity);
};

UINTERFACE(BlueprintType)
class EMBERCORE_API UEmberObjectiveParticipant : public UInterface { GENERATED_BODY() };

class EMBERCORE_API IEmberObjectiveParticipant
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ember|Mission")
    FName GetObjectiveParticipantId() const;
};

