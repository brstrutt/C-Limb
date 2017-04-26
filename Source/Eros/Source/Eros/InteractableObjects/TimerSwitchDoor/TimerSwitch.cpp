// Fill out your copyright notice in the Description page of Project Settings.

#include "Eros.h"
#include "TimerSwitch.h"
#include "TimerSwitchDoor.h"


ATimerSwitch::ATimerSwitch()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATimerSwitch::Initialise(ATimerSwitchDoor& DoorToActivate)
{
	Door = &DoorToActivate;
}

void ATimerSwitch::Activate()
{
	if (Door)
	{
		Door->OpenDoor(TimeOpenFor, this);
	}
}

