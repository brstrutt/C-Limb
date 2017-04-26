#pragma once

#include "OverlapComponents/ActorOverlapTrigger.h"
#include "CharacterVoicelineTrigger.generated.h"

class USoundCue;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EROS_API UCharacterVoicelineTrigger : public UActorOverlapTrigger
{
	GENERATED_BODY()
	
protected:

	virtual void OverlapBegin(AActor* OverlappedActor, AActor* Other) override;

private:

	UPROPERTY(EditInstanceOnly, Category = Voiceline)
	USoundCue* Voiceline;

	bool bHasActivated;
};
