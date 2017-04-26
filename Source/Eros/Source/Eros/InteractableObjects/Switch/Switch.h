#pragma once

#include "InteractableObjects/InteractableActor.h"
#include "Switch.generated.h"

class UGameCue;
class UWidgetComponent;

UCLASS(Abstract)
class EROS_API ASwitch : public AInteractableActor
{
	GENERATED_BODY()	

public:

	ASwitch();

	FORCEINLINE bool GetStartReversed() const { return bStartReversed; }

	virtual void BeginPlay() override;

	virtual bool Interact(AErosCharacter& Character) override;
	
	virtual void SetUIVisible(bool bShowUi) override;

	virtual void SetHighlightVisible(bool bShowHighlight) override;

	virtual void SetInteractable(bool bActive);

protected:

	/* Can the switch be activated just once, or many times. */
	UPROPERTY(EditAnywhere, Category = Switch)
	bool bSingleActivation;

	UPROPERTY(EditAnywhere, Category = Switch)
	TSubclassOf<UGameCue> GameCue;

	/* Set once the platform has activated once. */
	bool bHasActivated;

	/* Called when the switch is successfully activate. */
	virtual void Activate() {}

	/* Called prior to every activate, AFTER the first activation, to swap any neccessary variables.*/
	virtual void Swap() {};

private:


	/* Reverses the origin and destination. */
	UPROPERTY(EditAnywhere, Category = Switch)
	bool bStartReversed;

	UPROPERTY(EditDefaultsOnly, Category = Switch)
	UAudioComponent* AudioComponent;

	/* Mesh for the switch. */
	UPROPERTY(EditDefaultsOnly, Category = Platform)
	UStaticMeshComponent* Mesh;

	/* Box collider used to determine switch proximity */
	UPROPERTY(EditDefaultsOnly, Category = Switch)
	UBoxComponent* ProximityTrigger;

	UPROPERTY(EditDefaultsOnly, Category = Switch)
	USoundCue* InteractSound;
};
