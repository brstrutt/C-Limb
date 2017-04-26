#pragma once

#include "GameFramework/Actor.h"
#include "GrappleTarget.generated.h"

class UGrappleAnchor;

UCLASS()
class EROS_API AGrappleTarget : public AActor
{
	GENERATED_BODY()
	
public:	

	AGrappleTarget();

	virtual void BeginPlay() override;

	virtual UGrappleAnchor const* GetClosestAnchor(FVector const& CharacterLocation, FVector const& Point) const;

private:

	TArray<UGrappleAnchor*> Anchors;
};
