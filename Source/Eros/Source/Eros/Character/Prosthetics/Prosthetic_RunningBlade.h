#pragma once

#include "Prosthetic.h"
#include "Prosthetic_RunningBlade.generated.h"

struct FErosCharacterMovement;

/* A prosthetic for a leg socket that increases the characters speed at the cost of additional sound. */
UCLASS()
class EROS_API AProsthetic_RunningBlade : public AProsthetic
{
	GENERATED_BODY()

public:

	virtual FErosCharacterMovement ApplyModifiers(FErosCharacterMovement CharacterMovement) const override;

private:

	/* Amount to increase the characters speed by. */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetic, meta = (ClampMin = "1.0", ClampMax = "2.0", UIMin = "1.0", UIMax = "2.0"))
	float RunningSpeedModifier;
};
