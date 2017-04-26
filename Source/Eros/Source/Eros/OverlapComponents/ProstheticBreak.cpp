#include "Eros.h"

#include "../Character/ErosCharacter.h"
#include "ProstheticBreak.h"

void UProstheticBreak::OverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	AErosCharacter* Character = Cast<AErosCharacter>(OtherActor);

	if (!bHasActivated && Character != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Break those prosthetics!"));

		bHasActivated = true;
		Character->FuckIFellAndBrokeEverything();
	}
}