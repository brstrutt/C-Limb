// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InteractableObjects/Switch/Switch.h"
#include "DoorSwitch.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class EROS_API ADoorSwitch : public ASwitch
{
	GENERATED_BODY()

public:
	ADoorSwitch();

protected:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, Category=Door)
	void CloseDoor();
	void OpenDoor();

private:
	bool bIsOpen;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* Door;
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* DoorFrame;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent * CloseDoorTrigger;

	UPROPERTY(EditDefaultsOnly, Category = Switch)
	UAudioComponent* OpenCloseAudioComponent;
	UPROPERTY(EditDefaultsOnly, Category = Switch)
	USoundCue* OpenCloseSound;
};
