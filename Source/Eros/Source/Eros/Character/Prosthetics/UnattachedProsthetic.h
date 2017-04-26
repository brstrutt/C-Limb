#pragma once

#include "../../InteractableObjects/InteractableActor.h"
#include "ProstheticType.h"
#include "UnattachedProsthetic.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProstheticChanged);

UCLASS(Blueprintable)
class EROS_API AUnattachedProsthetic : public AInteractableActor
{
	GENERATED_BODY()
	
public:

	FOnProstheticChanged OnProstheticAttached;
	FOnProstheticChanged OnProstheticDetached;

	// Sets default values for this actor's properties
	AUnattachedProsthetic();

	/* Get the prosthetic type. */
	FORCEINLINE EProstheticType GetType() const { return ProstheticType; }

	/* Get the prosthetic reference. */
	FORCEINLINE TSubclassOf<AProsthetic> GetProsthetic() const { return AttachedProsthetic; }

	FORCEINLINE UStaticMeshComponent* GetMesh() { return ProstheticMesh; }
	
	virtual void SetHighlightVisible(bool bShowHighlight) override;
	
	/* Get the prosthetic type. */
	void CanSimulatePhysics(bool CanSimulatePhysics);
	
	void OnAttached();

	void OnDetached();

private:

	/* The type of this particular prosthetic. */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetic)
	EProstheticType ProstheticType;

	/* The attached version of this prosthetic. */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetic)
	TSubclassOf<AProsthetic> AttachedProsthetic;

	/* The prosthetic mesh... */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetic)
	UStaticMeshComponent* ProstheticMesh;

	/* Collider used by ErosCharacter to detect the prosthetic is nearby and thus can be picked up. */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetic)
	USceneComponent* PickupCollider;
};
