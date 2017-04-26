#include "Eros.h"

#include "Character/ErosCharacter.h"
#include "TriggerEnabler.h"

void UTriggerEnabler::BeginPlay()
{
	Super::BeginPlay();

	ActorToEnable->SetActorEnableCollision(false);
}

void UTriggerEnabler::OverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (bHasActivated) { return; }

	AErosCharacter* Character = Cast<AErosCharacter>(OtherActor);

	if (Character)
	{
		bHasActivated = true;

		ActorToEnable->SetActorEnableCollision(true);
	}
}