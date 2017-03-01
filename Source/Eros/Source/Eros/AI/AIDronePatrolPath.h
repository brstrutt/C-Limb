#pragma once

#include "GameFramework/Actor.h"
#include "AIDronePatrolPath.generated.h"

class USplineComponent;

UCLASS()
class EROS_API AAIDronePatrolPath : public AActor
{
	GENERATED_BODY()
	
public:	

	AAIDronePatrolPath();
	
	/* The spline describing an AI patrol route. */
	UPROPERTY(EditAnywhere, Category = "AI")
	USplineComponent* PatrolRouteSpline;	
};
