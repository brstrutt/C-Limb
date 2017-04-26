#include "Eros.h"

#include "MultiSwitch.h"
#include "MultiSwitchDoor.h"

AMultiSwitch::AMultiSwitch()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMultiSwitch::Initialise(AMultiSwitchDoor& DoorToActivate)
{
	Door = &DoorToActivate;
}

void AMultiSwitch::Activate()
{
	if (Door)
	{
		Door->SwitchActivated(*this);
	}
	if (CabinetToActivate)
	{
		CabinetToActivate->SetInteractable(true);
	}
}