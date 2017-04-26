#include "Eros.h"
#include "Character/ErosCharacter.h"
#include "FallDamageBlock.h"

void UFallDamageBlock::OverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	AErosCharacter* Character = Cast<AErosCharacter>(OtherActor);

	if (Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fall prevention"));

		Character->ResetFallDistance();
	}
}