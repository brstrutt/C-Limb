#include "Eros.h"

#include "../Character/ErosCharacter.h"
#include "../Managers/GameCuesManager.h"
#include "GameCueTrigger.h"

void UGameCueTrigger::OverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!bHasActivated && Cast<AErosCharacter>(OtherActor))
	{
		bHasActivated = true;

		AErosManager::GetManager<AGameCuesManager>(GetWorld())->PlayCue(*GameCue.GetDefaultObject());
	}
}