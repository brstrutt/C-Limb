// Fill out your copyright notice in the Description page of Project Settings.

#include "Eros.h"
#include "UnattachedProsthetic.h"
#include "Runtime/SlateCore/Public/Layout/SlateRect.h"
#include "Runtime/SlateCore/Public/Layout/ArrangedWidget.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"

// Sets default values
AUnattachedProsthetic::AUnattachedProsthetic()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = ProstheticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnattachedProstheticMesh"));
	SetHighlightVisible(true);

	ReparentToolTip(*ProstheticMesh);

	PickupCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("PickupCollider"));
	PickupCollider->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	CanSimulatePhysics(true);
}

void AUnattachedProsthetic::SetHighlightVisible(bool bShowHighlight)
{
	Super::SetHighlightVisible(bShowHighlight);

	ProstheticMesh->SetRenderCustomDepth(bShowHighlight);
}

void AUnattachedProsthetic::CanSimulatePhysics(bool CanSimulatePhysics)
{
	ProstheticMesh->SetSimulatePhysics(CanSimulatePhysics);
}

void AUnattachedProsthetic::OnAttached()
{
	OnProstheticAttached.Broadcast();
}

void AUnattachedProsthetic::OnDetached()
{
	OnProstheticDetached.Broadcast();
}