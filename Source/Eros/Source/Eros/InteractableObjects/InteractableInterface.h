#pragma once

#include "InteractableInterface.generated.h"

class AErosCharacter;
class AProsthetic;

UINTERFACE()
class EROS_API UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/* Interface for an object that can be interacted with. */
class EROS_API IInteractableInterface
{
	GENERATED_BODY()

public:

	/* Interact with the object, using the character. */
	virtual bool Interact(AErosCharacter& Character) = 0;

	/* Interact with the object, using a prosthetic. */
	virtual bool Interact(AProsthetic& Prosthetic) = 0;

	/* World location of the interactable object. */
	virtual FVector GetLocation() const = 0;

	/* Called when interaction has finished*/
	virtual bool StopInteract() = 0;
};