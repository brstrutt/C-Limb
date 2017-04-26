#pragma once

#include "OverlapComponents/ActorOverlapTrigger.h"
#include "CinematicTrigger.generated.h"

class ULevelSequence;
class ULevelSequencePlayer;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EROS_API UCinematicTrigger : public UActorOverlapTrigger
{
	GENERATED_BODY()

protected:
	
	virtual void OverlapBegin(AActor* OverlappedActor, AActor* OtherActor) override;

private:

	/* Level sequence to play showing the light change / door open. */
	UPROPERTY(EditAnywhere, Category = MultiSwitch)
	TArray<ULevelSequence*> Sequences;

	UPROPERTY(EditAnywhere, Category = Cinematic)
	bool bDisablePlayerInput;

	int SequenceIndex;

	/* Sequence player to play the level sequence. */
	UPROPERTY()	// Is this a fix to prevent garbage collections?
	ULevelSequencePlayer* Player;

	bool bHasActivated;

	UFUNCTION()
	void PlayNextSequence();
};
