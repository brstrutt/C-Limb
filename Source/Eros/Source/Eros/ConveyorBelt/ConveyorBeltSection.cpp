#include "Eros.h"
#include "ConveyorBeltSection.h"

AConveyorBeltSection::AConveyorBeltSection()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
}