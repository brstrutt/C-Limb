#pragma once

#include "OverlapComponents/ActorOverlapTrigger.h"
#include "FallDamageBlock.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EROS_API UFallDamageBlock : public UActorOverlapTrigger
{
	GENERATED_BODY()

protected:
		
	virtual void OverlapBegin(AActor* OverlappedActor, AActor* OtherActor) override;
};
