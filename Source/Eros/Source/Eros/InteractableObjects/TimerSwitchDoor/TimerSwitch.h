// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InteractableObjects/Switch/Switch.h"
#include "TimerSwitch.generated.h"


class ATimerSwitchDoor;

UCLASS(Blueprintable)
class EROS_API ATimerSwitch : public ASwitch
{
	GENERATED_BODY()

public:

	ATimerSwitch();

	/* Set the multi-switch door this switch is associated with. */
	void Initialise(ATimerSwitchDoor& DoorToActivate);

protected:

	virtual void Activate() override;

private:

	UPROPERTY(EditAnywhere, Category = TimerSwitch)
	ATimerSwitchDoor* Door;

	UPROPERTY(EditAnywhere, Category = TimerSwitch)
	float TimeOpenFor;
};
