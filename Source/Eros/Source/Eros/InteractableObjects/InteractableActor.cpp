#include "Eros.h"
#include "InteractableActor.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"

AInteractableActor::AInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

	ToolTip = CreateDefaultSubobject<UWidgetComponent>(TEXT("ToolTipWidget"));
	ToolTip->SetVisibility(false);
	ToolTip->SetWidgetSpace(EWidgetSpace::Screen);
	ToolTip->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
}

void AInteractableActor::BeginPlay()
{
	Super::BeginPlay();

	SetUIVisible(false);
	SetInteractable(bStartEnabled);
}

void AInteractableActor::SetUIVisible(bool bShowUi)
{
	if (bIsActive || !bShowUi)
	{
		ToolTip->SetVisibility(bShowUi);
	}
}

void AInteractableActor::SetInteractable(bool bActive)
{
	bIsActive = bActive;

	SetHighlightVisible(bActive);
}

void AInteractableActor::ReparentToolTip(USceneComponent& Parent)
{
	ToolTip->AttachToComponent(&Parent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
}