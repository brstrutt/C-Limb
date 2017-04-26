#include "Eros.h"

#include "CharacterVoicelineTrigger.h"
#include "Character/ErosCharacter.h"
#include "Sound/SoundCue.h"

void UCharacterVoicelineTrigger::OverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	AErosCharacter* Character = Cast<AErosCharacter>(OtherActor);

	if (!bHasActivated && IsValid(Character))
	{
		bHasActivated = true;

		Character->Say(Voiceline);
	}
}