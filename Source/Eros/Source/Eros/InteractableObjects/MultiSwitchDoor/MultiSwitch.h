#pragma once

#include "../Switch/Switch.h"
#include "MultiSwitch.generated.h"

class AMultiSwitchDoor;

UCLASS(Blueprintable)
class EROS_API AMultiSwitch : public ASwitch
{
	GENERATED_BODY()
	
public:	

	AMultiSwitch();

	/* Set the multi-switch door this switch is associated with. */
	void Initialise(AMultiSwitchDoor& DoorToActivate);

protected:

	virtual void Activate() override;

private:

	AMultiSwitchDoor* Door;

	UPROPERTY(EditAnywhere, category = Switch)
	AProstheticCabinet* CabinetToActivate;
};
