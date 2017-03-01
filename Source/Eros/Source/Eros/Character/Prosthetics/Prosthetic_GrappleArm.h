#pragma once

#include "Prosthetic.h"
#include "Prosthetic_GrappleArm.generated.h"

/* A prosthetic arm that allows the player grapple over gaps. */
UCLASS()
class EROS_API AProsthetic_GrappleArm : public AProsthetic
{
	GENERATED_BODY()

public:

	AProsthetic_GrappleArm();

	virtual void PrimaryActionBegin() override;

private:

	/* Length of Ray for grapple to extend. */
	UPROPERTY(EditDefaultsOnly, Category = "Grapple Settings")
	float RayLength;

	/* Speed at which the grapple retracts the user to the target location. */
	UPROPERTY(EditDefaultsOnly, Category = "Grapple Settings")
	float GrappleSpeed;
};