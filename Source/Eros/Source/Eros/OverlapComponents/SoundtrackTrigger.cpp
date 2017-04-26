#include "Eros.h"

#include "Character/ErosCharacter.h"
#include "Managers/AudioManager.h"
#include "SoundtrackTrigger.h"

void USoundtrackTrigger::OverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	AErosCharacter* Character = Cast<AErosCharacter>(OtherActor);

	if (!bHasActivated && Character != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Changing soundtrack!"));

		bHasActivated = true;

		AErosManager::GetManager<AAudioManager>(GetWorld())->PlaySong(Song);
	}
}