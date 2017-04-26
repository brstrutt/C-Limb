#include "Eros.h"

#include "Character/ErosCharacter.h"
#include "Misc/SpacedSoundCue.h"
#include "SpacedSoundTrigger.h"

void USpacedSoundTrigger::OverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (bHasActivated) { return; }

	AErosCharacter* Character = Cast<AErosCharacter>(OtherActor);

	if (Character)
	{
		bHasActivated = true;

		Sound->Enable();
	}
}