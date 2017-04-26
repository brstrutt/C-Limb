#pragma once

#include "OverlapComponents/ActorOverlapTrigger.h"
#include "Checkpoint.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EROS_API UCheckpoint : public UActorOverlapTrigger
{
	GENERATED_BODY()

protected:

	virtual void OverlapBegin(AActor* OverlappedActor, AActor* OtherActor) override;

private:

	bool bHasActivated;
};
