#pragma once

#include "GameFramework/Actor.h"
#include "InteractableActor.generated.h"

class UWidgetComponent;

UCLASS(Abstract, NotBlueprintable)
class EROS_API AInteractableActor : public AActor
{
	GENERATED_BODY()
	
public:	

	AInteractableActor();

	virtual void BeginPlay() override;

	virtual bool Interact(AErosCharacter& Character) { return false; }

	virtual bool Interact(AProsthetic& Prosthetic) { return false; }

	virtual bool StopInteract() { return false; }

	virtual void SetHighlightVisible(bool bShowHighlight) {};

	virtual void SetUIVisible(bool bShowUI);

	virtual void SetInteractable(bool bActive);

	/* The tooltip is added to the root component in the constructer, but may likely want re-attaching in derived classes. */
	virtual void ReparentToolTip(USceneComponent& Parent);

	bool GetIsActive() { return bIsActive; }

protected:

	UPROPERTY(EditAnywhere, Category = Interactable)
	bool bStartEnabled = true;

private:

	/* Tool tip widget. */
	UPROPERTY(EditDefaultsOnly, Category = UI)
	UWidgetComponent* ToolTip;

	bool bIsActive;
};
