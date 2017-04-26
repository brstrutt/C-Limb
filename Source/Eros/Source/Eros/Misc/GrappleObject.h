#pragma once

#include "Character/ErosCharacter.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "GrappleObject.generated.h"

UCLASS()
class EROS_API AGrappleObject : public AActor
{
	GENERATED_BODY()
	
public:	

	AGrappleObject();

	FORCEINLINE void SetCharacter(AErosCharacter& Char) { Character = &Char; }

	FORCEINLINE USplineMeshComponent* GetRope() { return SplineRope; }

	virtual void BeginPlay() override;
	
	virtual void Tick( float DeltaSeconds ) override;

	void SetGrappleTarget(const FVector& End);

private:
	UPROPERTY(EditAnywhere, Category = "Grapple Rope")
	USplineComponent* SplineComp;

	UPROPERTY(EditAnywhere, Category = "Grapple Rope")
	USplineMeshComponent* SplineRope;

	FVector StartLocation;
	FVector EndLocation;
	FVector StartTangent;
	FVector EndTangent;

	AErosCharacter* Character;
};
