#include "Eros.h"

#include "DrawDebugHelpers.h"
#include "GrappleObject.h"

AGrappleObject::AGrappleObject()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = SplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	SplineRope = CreateDefaultSubobject<USplineMeshComponent>(TEXT("Spline"));
	SplineRope->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
}

void AGrappleObject::BeginPlay()
{
	Super::BeginPlay();

	SplineRope->SetVisibility(false);
}

void AGrappleObject::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime);

	if (!Character) { return; }

	SplineComp->SetLocationAtSplinePoint(0, Character->GetMesh()->GetSocketLocation(FName("LFT_Digit_04_Base_Bone")), ESplineCoordinateSpace::World);

	SplineComp->GetLocalLocationAndTangentAtSplinePoint(0, StartLocation, StartTangent);
	SplineComp->GetLocalLocationAndTangentAtSplinePoint(SplineComp->GetNumberOfSplinePoints() - 1, EndLocation, EndTangent);
	
	SplineRope->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent);

	SplineRope->UpdateBounds();

}

void AGrappleObject::SetGrappleTarget(const FVector& End)
{
	SplineComp->SetLocationAtSplinePoint(SplineComp->GetNumberOfSplinePoints() - 1, End, ESplineCoordinateSpace::World);
}

