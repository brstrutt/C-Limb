#include "Eros.h"
#include "ActorRotate.h"

UActorRotate::UActorRotate()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UActorRotate::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	GetOwner()->AddActorLocalRotation(Angle * DeltaTime);
}

