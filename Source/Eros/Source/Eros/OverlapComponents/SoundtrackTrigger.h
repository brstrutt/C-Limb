#pragma once

#include "OverlapComponents/ActorOverlapTrigger.h"
#include "SoundtrackTrigger.generated.h"

class USoundCue;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EROS_API USoundtrackTrigger : public UActorOverlapTrigger
{
	GENERATED_BODY()
	
private:

	UPROPERTY(EditAnywhere, Category = Soundtrack)
	USoundCue* Song;

	bool bHasActivated;

	virtual void OverlapBegin(AActor* OverlappedActor, AActor* OtherActor) override;
};
