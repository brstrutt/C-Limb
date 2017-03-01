#pragma once

#include "InteractableObjects/InteractableActor.h"
#include "Switch.generated.h"

UCLASS(Abstract)
class EROS_API ASwitch : public AInteractableActor
{
	GENERATED_BODY()	

public:

	virtual bool Interact(AErosCharacter& Character) override;

	FORCEINLINE bool GetStartReversed() const { return bStartReversed; }

protected:

	/* Called when the switch is successfully activate. */
	virtual void Activate() {}

	/* Called prior to every activate, AFTER the first activation, to swap any neccessary variables.*/
	virtual void Swap() {};

private:

	/* Can the switch be activated just once, or many times. */
	UPROPERTY(EditAnywhere, Category = Switch)
	bool bSingleActivation;

	/* Reverses the origin and destination. */
	UPROPERTY(EditAnywhere, Category = Switch)
	bool bStartReversed;

	/* Set once the platform has activated once. */
	bool bHasActivated;
};
