#pragma once

#include "OverlapComponents/ActorOverlapTrigger.h"
#include "GameCueTrigger.generated.h"

class UGameCue;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EROS_API UGameCueTrigger : public UActorOverlapTrigger
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, Category = Cue)
	TSubclassOf<UGameCue> GameCue;

	bool bHasActivated;

	virtual void OverlapBegin(AActor* OverlappedActor, AActor* OtherActor) override;
};
