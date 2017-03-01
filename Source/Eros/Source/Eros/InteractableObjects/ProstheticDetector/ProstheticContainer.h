#pragma once

#include "../InteractableActor.h"
#include "ProstheticContainer.generated.h"

class AProsthetic;

UCLASS(Blueprintable)
class EROS_API AProstheticContainer : public AInteractableActor
{
	GENERATED_BODY()
	
public:	

	AProstheticContainer();

	virtual bool Interact(AErosCharacter& Character) override;

	/* Has the player placed the required prosthetic. */
	FORCEINLINE bool HasProsthetic() const { return bHasProsthetic; }

	/* Get the UClass representing the accepted prosthetic type. */
	FORCEINLINE UClass* GetAcceptedProsthetic() const { return AcceptedProsthetic.GetDefaultObject()->GetClass(); }

private:

	/* Mesh of the container. */
	UPROPERTY(EditDefaultsOnly, Category = "Prosthetic Container")
	UStaticMeshComponent* Mesh;

	/* Type of prosthetic that can be stored in the container. */
	UPROPERTY(EditAnywhere, Category = "Prosthetic Container")
	TSubclassOf<AProsthetic> AcceptedProsthetic;

	/* Is there a prosthetic in the container. */
	bool bHasProsthetic;
};
