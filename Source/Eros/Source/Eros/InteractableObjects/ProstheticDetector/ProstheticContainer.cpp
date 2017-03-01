#include "Eros.h"

#include "../../Character/ErosCharacter.h"
#include "../../Character/Prosthetics/Prosthetic.h"
#include "ProstheticContainer.h"

AProstheticContainer::AProstheticContainer()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
}

bool AProstheticContainer::Interact(AErosCharacter& Character)
{
	if (Character.HasProstheticAttached(AcceptedProsthetic.GetDefaultObject()->GetClass()))
	{
		bHasProsthetic = true;

		GetWorld()->DestroyActor(Character.DetachProstheticOfType(AcceptedProsthetic.GetDefaultObject()->GetType()));

		UE_LOG(LogTemp, Warning, TEXT("Prosthetic place!"));

		return true;
	}

	return false;
}