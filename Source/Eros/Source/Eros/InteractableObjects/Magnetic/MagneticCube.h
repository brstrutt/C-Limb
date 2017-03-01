#pragma once

#include "../InteractableActor.h"
#include "MagneticCube.generated.h"

UCLASS()
class EROS_API AMagneticCube : public AInteractableActor
{
	GENERATED_BODY()
	
public:	

	AMagneticCube();

	virtual bool Interact(AProsthetic& Prosthetic) override;

	virtual bool StopInteract() override;

private:

	UPROPERTY(EditDefaultsOnly, Category = Interactable)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, Category = Interactable)
	UStaticMeshComponent* Collider;
};
