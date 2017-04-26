#pragma once

#include "OverlapComponents/ActorOverlapTrigger.h"
#include "TutorialTrigger.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EROS_API UTutorialTrigger : public UActorOverlapTrigger
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, Category = Tutorial)
	bool bActivateTutorial;

	bool bHasActivated;

	virtual void OverlapBegin(AActor* OverlappedActor, AActor* OtherActor) override;
};
