#include "Eros.h"

#include "Character/ErosCharacter.h"
#include "InteractableObjects/InteractableActor.h"
#include "InteractableTrigger.h"

void UInteractableTrigger::OverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!bHasActivated && Cast<AErosCharacter>(OtherActor))
	{
		bHasActivated = true;

		for (int Index = 0; Index < ToEnable.Num(); Index++)
		{
			ToEnable[Index]->SetInteractable(true);
		}

		for (int Index = 0; Index < ToDisable.Num(); Index++)
		{
			ToDisable[Index]->SetInteractable(false);
		}
	}
}