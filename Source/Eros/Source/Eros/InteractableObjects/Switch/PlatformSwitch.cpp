#include "Eros.h"

#include "../../Character/ErosCharacter.h"
#include "PlatformSwitch.h"

APlatformSwitch::APlatformSwitch()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
}

void APlatformSwitch::BeginPlay()
{
	Super::BeginPlay();

	if (Platform == nullptr) { return; }

	if (GetStartReversed())
	{
		Destination = Platform->GetActorLocation();
		Origin = Destination - MovementDistance;
	}
	else
	{
		Origin = Platform->GetActorLocation();
		Destination = Origin + MovementDistance;
	}

	Platform->SetActorLocation(Origin);

	SetActorTickEnabled(false);
}

void APlatformSwitch::Activate()
{
	SetActorTickEnabled(true);
}

void APlatformSwitch::Swap()
{
	FVector Temp = Destination;
	Destination = Origin;
	Origin = Temp;
}

void APlatformSwitch::Tick(float DeltaSeconds)
{
	if (Platform == nullptr) { return; }

	if (Platform->GetActorLocation() == Destination)
	{
		// Arrived at our destination.
		SetActorTickEnabled(false);
		return;
	}

	FVector DistanceToDestination = Destination - Platform->GetActorLocation();
	FVector DistanceToMove = DistanceToDestination.GetUnsafeNormal() * MovementSpeed * DeltaSeconds;

	if (DistanceToMove.SizeSquared() > DistanceToDestination.SizeSquared())
	{
		DistanceToMove = DistanceToDestination;
	}

	Platform->AddActorWorldOffset(DistanceToMove);
}