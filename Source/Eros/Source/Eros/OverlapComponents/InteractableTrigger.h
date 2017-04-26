#pragma once

#include "OverlapComponents/ActorOverlapTrigger.h"
#include "InteractableTrigger.generated.h"

class AInteractableActor;

/* Trigger to enable or disable interactable actors. */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EROS_API UInteractableTrigger : public UActorOverlapTrigger
{
	GENERATED_BODY()
	
protected:

	virtual void OverlapBegin(AActor* OverlappedActor, AActor* OtherActor) override;

private:

	UPROPERTY(EditAnywhere, Category = Trigger)
	TArray<AInteractableActor*> ToEnable;

	UPROPERTY(EditAnywhere, Category = Trigger)
	TArray<AInteractableActor*> ToDisable;

	bool bHasActivated;
};
