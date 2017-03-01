#pragma once

#include "Prosthetic.h"
#include "Prosthetic_SpringLeg.generated.h"

struct FErosCharacterMovement;

/* A prosthetic for a leg socket that increases the jump force of the player. */
UCLASS()
class EROS_API AProsthetic_SpringLeg : public AProsthetic
{
	GENERATED_BODY()

public:

	virtual FErosCharacterMovement ApplyModifiers(FErosCharacterMovement CharacterMovement) const override;

private:

	/* Amount to increase the jump force by. */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetic, meta = (ClampMin = "1.0", ClampMax = "2.0", UIMin = "1.0", UIMax = "2.0"))
	float JumpForceModifier;

	UPROPERTY(EditDefaultsOnly, Category = Prosthetic, meta = (ClampMin = "0.5", ClampMax = "1.0", UIMin = "0.5", UIMax = "1.0"))
	float RunSpeedModifier;
};
