#include "Eros.h"

#include "../Character/ErosCharacter.h"
#include "../Character/ErosController.h"
#include "TutorialTrigger.h"

void UTutorialTrigger::OverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	AErosCharacter* Character = Cast<AErosCharacter>(OtherActor);

	if (!bHasActivated && Character != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tutorial enabled/disabled!"));

		bHasActivated = true;

		Cast<AErosController>(Character->GetController())->SetShowProstheticTutorials(bActivateTutorial);
	}
}