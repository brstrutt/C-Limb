#include "Eros.h"

#include "AIDronePatrolPath.h"
#include <Components/SplineComponent.h>

AAIDronePatrolPath::AAIDronePatrolPath()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create the default spline
	PatrolRouteSpline = CreateDefaultSubobject<USplineComponent>(TEXT("Drone Patrol Route"));
	PatrolRouteSpline->SetClosedLoop(true);
	PatrolRouteSpline->SetSplinePointType(0, ESplinePointType::Linear);
	PatrolRouteSpline->SetSplinePointType(1, ESplinePointType::Linear);
}