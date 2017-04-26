// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "TimerSwitchDoor.generated.h"

class ATimerSwitch;

UCLASS()
class EROS_API ATimerSwitchDoor : public AActor
{
	GENERATED_BODY()

		friend ATimerSwitch;

public:

	ATimerSwitchDoor();

	virtual void BeginPlay() override;

private:
	bool bIsOpen;

	UPROPERTY(EditAnywhere, Category = TimerDoor)
	float OpenHeight;

	UPROPERTY(EditAnywhere, Category = BrokenDoor)
	UParticleSystemComponent* Sparks;

	UPROPERTY(EditAnywhere, Category = TimerDoor)
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(EditDefaultsOnly, Category = TimerDoor)
	UAudioComponent* OpenCloseAudioComponent;
	UPROPERTY(EditDefaultsOnly, Category = TimerDoor)
	USoundCue* OpenCloseSound;

	/* This switch that last activated this door. */
	ASwitch* ControllingSwitch;

	void OpenDoor(float TimeOpenFor, ASwitch* NewControllingSwitch);
	void CloseDoor();
};
