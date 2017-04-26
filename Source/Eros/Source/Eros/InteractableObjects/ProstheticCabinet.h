#pragma once

#include "InteractableObjects/InteractableActor.h"
#include "ProstheticCabinet.generated.h"

class ASwitch;
class AUnattachedProsthetic;
class UGameCue;

UCLASS(Blueprintable)
class EROS_API AProstheticCabinet : public AInteractableActor
{
	GENERATED_BODY()

public:
	AProstheticCabinet();

	virtual void BeginPlay() override;

	virtual bool Interact(AErosCharacter& Character) override;

	virtual void SetUIVisible(bool bShowUi) override;

	virtual void SetHighlightVisible(bool bShowHighlight) override;

	virtual void SetInteractable(bool bActive) override;

private:

	/* The cabinet is only interactable when this is true. */
	UPROPERTY(EditAnywhere)
	bool bActive;

	UPROPERTY(EditAnywhere)
	bool bIsTutorial;

	UPROPERTY(EditAnywhere)
	ASwitch* SwitchToActivate;

	UPROPERTY(EditDefaultsOnly)
	USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = Prosthetic)
	TSubclassOf<AUnattachedProsthetic> ProstheticToSpawn;

	UPROPERTY(EditAnywhere, Category = Prosthetic)
	TSubclassOf<UGameCue> GameCue;

	AUnattachedProsthetic* ProstheticInstance;

	bool bIsOpen = false;

	UFUNCTION()
	void ProstheticAttached();
};
