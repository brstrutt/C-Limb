#pragma once

#include "../ErosCharacter.h"
#include "Prosthetic.h"
#include "Prosthetic_GrappleArm.generated.h"

class AGrappleAnchorReticle;
class UCableComponent;
class UGrappleAnchor;

/* A prosthetic arm that allows the player grapple over gaps. */
UCLASS()
class EROS_API AProsthetic_GrappleArm : public AProsthetic_BasicArm
{
	GENERATED_BODY()

public:

	AProsthetic_GrappleArm();
	~AProsthetic_GrappleArm();

	virtual void PrimaryActionBegin() override;
	virtual void PrimaryActionEnd() override;

	virtual void BeginPlay() override;

	FORCEINLINE float GetRayMax() { return RayMax; }

	virtual void SetMeshVisibility(bool bShow) override;

protected:

	virtual void Tick(float DeltaSeconds) override;

	virtual void OnAttached() override;

private:

	/* Length of Ray for grapple to extend. */
	UPROPERTY(EditDefaultsOnly, Category = Grapple)
	float RayMax;

	/* Speed at which the grapple retracts the user to the target location. */
	UPROPERTY(EditDefaultsOnly, Category = Grapple)
	float GrappleSpeed;

	/* Fuck the cable component. */
	UPROPERTY(EditDefaultsOnly, Category = Grapple)
	UCableComponent* Cable;

	/* Blueprint class of the invisible cable head. */
	UPROPERTY(EditDefaultsOnly, Category = Grapple)
	TSubclassOf<AActor> InvisibleCableHeadTemplate;

	UPROPERTY(EditDefaultsOnly, Category = Grapple)
	TSubclassOf<AGrappleAnchorReticle> GrappleReticleTemplate;

	/* Instance of the cable head. */
	AActor* InvisibleCableHead;

	/* Instance of the grapple reticle. */
	AGrappleAnchorReticle* GrappleReticle;

	/* Character this prosthetic is attached to. */
	AErosCharacter* Character;

	/* Anchor the grapple is grappling to. */
	UGrappleAnchor const* TargetAnchor;

	/* Is the grapple grappling towards the TargetAnchor. */
	bool IsHooking = false;

	/* Is the grapple cable extending towards the TargetAnchor. */
	bool CanHook = false;

	/* Timer used to disable the grapple cable.*/
	float Timer;

	/* Finds the closest grapple anchor the character can grapple to. May be null. */
	UGrappleAnchor const* GetClosestAnchor() const;
};