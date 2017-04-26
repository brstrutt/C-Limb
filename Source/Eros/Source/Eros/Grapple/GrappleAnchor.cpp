#include "Eros.h"
#include "GrappleAnchor.h"
#include "Runtime/Engine/Classes/Components/ArrowComponent.h"

UGrappleAnchor::UGrappleAnchor()
{
	PrimaryComponentTick.bCanEverTick = false;

	ForwardArrow = CreateDefaultSubobject<UArrowComponent>(FName(*("Forward_" + this->GetName())));
	ForwardArrow->AttachToComponent(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	MaxAngle = 85.0f;
}

bool UGrappleAnchor::IsValid(FVector const& Direction) const
{
	return MaxAngle >= FMath::RadiansToDegrees(acosf(FVector::DotProduct(Direction.GetUnsafeNormal(), GetForwardVector())));
}