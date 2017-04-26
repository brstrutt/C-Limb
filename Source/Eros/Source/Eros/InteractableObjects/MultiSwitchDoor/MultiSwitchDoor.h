#pragma once

#include "GameFramework/Actor.h"
#include "MultiSwitchDoor.generated.h"

class AMultiSwitch; 
class UGameCue;
class ULevelSequence;
class ULevelSequencePlayer;

/* All the components neccessary for a multi door switch. */
USTRUCT()
struct EROS_API FSwitchObject
{
	GENERATED_BODY()

	/* Associated switch object. */
	UPROPERTY(EditAnywhere, Category = MultiSwitch)
	AMultiSwitch* Switch;

	/* Actor to show when the switch has not been activated, most likely a light. */
	UPROPERTY(EditAnywhere, Category = MultiSwitch)
	AActor* InactiveActor;

	/* Actor to show when the switch has been activated, most likely a light. */
	UPROPERTY(EditAnywhere, Category = MultiSwitch)
	AActor* ActiveActor;

	/* Game cue to play after the sequence of the light playing has completed. */
	UPROPERTY(EditAnywhere, Category = MultiSwitch)
	TSubclassOf<UGameCue> SequenceCompleteCue;

	/* Has the switch been activated. */
	bool bActive;
};

/* A multi-door switch is a door that requires multiple switches to active. */
UCLASS(Blueprintable)
class EROS_API AMultiSwitchDoor : public AActor
{
	GENERATED_BODY()

	friend AMultiSwitch;

public:	

	AMultiSwitchDoor();

	virtual void BeginPlay() override;

	/* 
	 * Attempt to unlock the door.
	 *
	 * Returns: true if the door unlocked or was already unlocked.
	 */
	bool TryUnlock();

protected:



private:

	/* All the switches required to activate the door. */
	UPROPERTY(EditAnywhere, Category = MultiSwitch)
	TArray<FSwitchObject> Switches;

	/* Temporary door object to remove when all switches have been activated. */
	UPROPERTY(EditAnywhere, Category = MultiSwitch)
	UStaticMeshComponent* TemporaryDoor;

	/* Level sequence to play showing the light change / door open. */
	UPROPERTY(EditAnywhere, Category = MultiSwitch)
	ULevelSequence* Sequence;

	/* Sequence player to play the level sequence. */
	UPROPERTY() // Bug fix for cinematic???
	ULevelSequencePlayer* Player;

	UPROPERTY(EditDefaultsOnly, Category = MultiSwitch)
		UAudioComponent* OpenCloseAudioComponent;
	UPROPERTY(EditDefaultsOnly, Category = MultiSwitch)
		USoundCue* OpenCloseSound;

	/* Used in the timer to change a light. */
	FTimerHandle TimerHandle;
	/* The switch object to change after a short delay. */
	FSwitchObject* ToActivate;

	/* Is the door unlocked... */
	bool bUnlocked;

	UGameCue* GameCueToPlay;

	/* Function called when a switch is activated. */
	void SwitchActivated(AMultiSwitch& Switch);

	/* Called after a short delay to activate a light. */
	UFUNCTION()
	void DelayedActivation();

	UFUNCTION()
	void SequenceComplete();
};
