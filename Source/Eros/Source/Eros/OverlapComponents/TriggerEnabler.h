#pragma once

#include "OverlapComponents/ActorOverlapTrigger.h"
#include "TriggerEnabler.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EROS_API UTriggerEnabler : public UActorOverlapTrigger
{
	GENERATED_BODY()
	
public:

	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere)
	AActor* ActorToEnable;
		
	bool bHasActivated;

	virtual void OverlapBegin(AActor* OverlappedActor, AActor* OtherActor) override;
};
