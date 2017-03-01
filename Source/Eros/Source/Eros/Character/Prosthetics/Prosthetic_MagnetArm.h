#pragma once

#include "Prosthetic.h"
#include "Prosthetic_MagnetArm.generated.h"

class IInteractableInterface;

/* An arm prosthetic that allows the player to move magnetic objects. */
UCLASS()
class EROS_API AProsthetic_MagnetArm : public AProsthetic
{
	GENERATED_BODY()

public:

	AProsthetic_MagnetArm();
		
	/* Strength of the magnet. */
	FORCEINLINE float GetMagnetStrength() const { return MagnetStrength; }

	/* Exposed raycast start. */
	FORCEINLINE FVector GetCurrentMagneticRayStart() const { return CurrentMagneticRayStart; }

	/* Exposed raycast end. */
	FORCEINLINE FVector GetCurrentMagneticRayEnd() const { return CurrentMagneticRayEnd; }

protected:

	virtual void Tick(float DeltaSeconds) override;

	/* Push or pull a magnetic object. */
	virtual void Magnetize(bool IsPull);

private:

	/* Length of the ray cast for pushing or pulling magnetic objects. */
	UPROPERTY(EditDefaultsOnly, Category = "Magnet Settings")
	float RayLength;

	/* Force multiplier for pushing or pulling magnetic objects. */
	UPROPERTY(EditDefaultsOnly, Category = "Magnet Settings")
	float MagnetStrength;

	FVector CurrentMagneticRayStart;
	
	FVector CurrentMagneticRayEnd;

	/* Interactable being pushed or pulled. */
	IInteractableInterface* Interactable;
};