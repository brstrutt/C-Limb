#pragma once

#include "Components/ActorComponent.h"
#include "ActorOverlapTrigger.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EROS_API UActorOverlapTrigger : public UActorComponent
{
	GENERATED_BODY()

public:

	UActorOverlapTrigger();

	virtual void BeginPlay() override;

protected:

	virtual void OverlapBegin(AActor* OverlappedActor, AActor* OtherActor) { }

	virtual void OverlapEnd(AActor* OverlappedActor, AActor* OtherActor) { }

private:
	
	UFUNCTION()
	void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);
	UFUNCTION()
	void OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor);
};
