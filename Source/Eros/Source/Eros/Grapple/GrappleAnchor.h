#pragma once

#include "Components/SceneComponent.h"
#include "GrappleAnchor.generated.h"

class UArrowComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EROS_API UGrappleAnchor : public USceneComponent
{
	GENERATED_BODY()

public:	

	UGrappleAnchor();

	FORCEINLINE float GetMaxAngle() const { return MaxAngle; }

	/* Checks if a direction vector to this anchor is acceptable. */
	virtual bool IsValid(FVector const& Direction) const;

private:

	UPROPERTY()
	UArrowComponent* ForwardArrow;

	/* Maximum angle allowed to grapple to this anchor. */
	UPROPERTY(EditAnywhere, Category = Anchor)
	float MaxAngle;
};
