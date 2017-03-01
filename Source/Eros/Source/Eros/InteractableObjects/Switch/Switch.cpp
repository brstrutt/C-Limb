#include "Eros.h"

#include "Switch.h"

bool ASwitch::Interact(AErosCharacter& Character)
{
	if (!bSingleActivation || !bHasActivated)
	{
		UE_LOG(LogTemp, Warning, TEXT("INTERACT"));

		if (bHasActivated)
		{
			Swap();
		}

		bHasActivated = true;

		Activate();

		return true;
	}

	return false;
}