#pragma once

#include "OverlapComponents/ActorOverlapTrigger.h"
#include "ProstheticBreak.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EROS_API UProstheticBreak : public UActorOverlapTrigger
{
	GENERATED_BODY()
	
private:

	bool bHasActivated;

	virtual void OverlapBegin(AActor* OverlappedActor, AActor* OtherActor) override;	
};
