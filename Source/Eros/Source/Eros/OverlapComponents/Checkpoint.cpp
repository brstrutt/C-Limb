#include "Eros.h"

#include "../Character/ErosCharacter.h"
#include "Checkpoint.h"
#include "Misc/ErosGameMode.h"

void UCheckpoint::OverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	AErosCharacter* TempCharacter = Cast<AErosCharacter>(OtherActor);

	if (!bHasActivated && TempCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Checkpoint!"));

		bHasActivated = true;
		Cast<AErosGameMode>(GetWorld()->GetAuthGameMode())->UpdateRespawnInformation(OverlappedActor, TempCharacter);
	}
}