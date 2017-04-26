#pragma once

#include "OverlapComponents/ActorOverlapTrigger.h"
#include "SpacedSoundTrigger.generated.h"

class ASpacedSoundCue;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EROS_API USpacedSoundTrigger : public UActorOverlapTrigger
{
	GENERATED_BODY()
	
private:

	UPROPERTY(EditAnywhere)
	ASpacedSoundCue* Sound;

	bool bHasActivated;
		
	virtual void OverlapBegin(AActor* OverlappedActor, AActor* OtherActor) override;	
};
