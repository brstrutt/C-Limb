#include "Eros.h"

#include "GrappleAnchor.h"
#include "GrappleTarget.h"

AGrappleTarget::AGrappleTarget()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGrappleTarget::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<UActorComponent*> GrappleComponents = GetComponentsByClass(UGrappleAnchor::StaticClass());

	for (int Index = 0; Index < GrappleComponents.Num(); Index++)
	{
		Anchors.Add(static_cast<UGrappleAnchor*>(GrappleComponents[Index]));
	}
}

UGrappleAnchor const* AGrappleTarget::GetClosestAnchor(FVector const& CharacterLocation, FVector const& Point) const
{
	float ClosestDistance = 0.0f;
	UGrappleAnchor* ClosestAnchor = nullptr;

	for (int Index = 0; Index < Anchors.Num(); Index++)
	{
		// Direction of the character to the anchor must be valid.
		FVector const Direction = (CharacterLocation - Anchors[Index]->GetComponentLocation()).GetUnsafeNormal();

		if (!Anchors[Index]->IsValid(Direction)) { continue; }

		// Check if this is the closest valid anchor.
		float const Distance = (Point - Anchors[Index]->GetComponentLocation()).Size();

		if (ClosestAnchor == nullptr || Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestAnchor = Anchors[Index];
		}
	}

	return ClosestAnchor;
}