#pragma once

#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "InteractableActor.generated.h"

UCLASS(Abstract, NotBlueprintable)
class EROS_API AInteractableActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	

	AInteractableActor();

	virtual bool Interact(AErosCharacter& Character) override { return false; }

	virtual bool Interact(AProsthetic& Prosthetic) override { return false; }

	virtual FVector GetLocation() const override { return GetActorLocation(); }

	virtual bool StopInteract() override { return false; }

protected:

	virtual void Tick(float DeltaSeconds) override { Super::Tick(DeltaSeconds); };
};
