#pragma once

#include "InteractableObjects/Switch/Switch.h"
#include "FactorySwitch.generated.h"

class ALight;
class ASkeletalMeshActor;
class ULevelSequence;
class ULevelSequencePlayer;

UCLASS(Blueprintable)
class EROS_API AFactorySwitch : public ASwitch
{
	GENERATED_BODY()
	
public:

	AFactorySwitch();

	virtual void BeginPlay() override;
	
protected:
	virtual void Activate() override;
	
private:

	/* A sequence to play... */
	UPROPERTY(EditAnywhere, Category = Cinematics)
	ULevelSequence* Sequence;

	UPROPERTY(EditAnywhere, Category = ControlledActors)
	TArray<ALight*> ControlledLights; 
	UPROPERTY(EditAnywhere, Category = ControlledActors)
	TArray<ASkeletalMeshActor*> ControlledArms;
	UPROPERTY(EditAnywhere, Category = ControlledActors)
	TArray<AConveyorBelt*> ConveyorBeltControllers;

	/* Sequence player to play the level sequence. */
	UPROPERTY()	// Is this a fix to prevent garbage collections?
	ULevelSequencePlayer* Player;

	UFUNCTION()
	void SequenceDone();

	void SetLightsActive(bool Activate);
	void SetAnimationsActive(bool Activate);
	void SetConveyorBeltActive(bool Activate);
};
