#include "Eros.h"
#include "GrappleAnchor.h"
#include "GrappleAnchorReticle.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"

AGrappleAnchorReticle::AGrappleAnchorReticle()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Reticle = CreateDefaultSubobject<UWidgetComponent>(TEXT("ToolTipWidget"));
	Reticle->SetVisibility(true);
	Reticle->SetWidgetSpace(EWidgetSpace::Screen);
	Reticle->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
}

void AGrappleAnchorReticle::BeginPlay()
{
	Super::BeginPlay();
	
	Reticle->SetVisibility(false);
	UpdateFadeValue(0.0f);
}

void AGrappleAnchorReticle::Tick(float DeltaTime)
{
	Super::Tick( DeltaTime );

	if (bFadeIn)
	{
		ReticleFadeValue += (1.0f / FadeTime) * DeltaTime;

		if (ReticleFadeValue >= 1.0f)
		{
			bFadeIn = false;
		}

		UpdateFadeValue(ReticleFadeValue);
	}
	else if (bFadeOut)
	{
		ReticleFadeValue -= (1.0f / FadeTime) * DeltaTime;

		if (ReticleFadeValue <= 0.0f)
		{
			Target = nullptr;
			bFadeOut = false;

			Reticle->SetVisibility(false); 
		}

		UpdateFadeValue(ReticleFadeValue);
	}
}

void AGrappleAnchorReticle::SetTarget(UGrappleAnchor const* Anchor)
{
	if (Target == Anchor || !IsValid(Anchor)) { return; }
	
	Reticle->SetVisibility(true);

	if (ReticleFadeValue != 1.0f)
	{
		bFadeIn = true;
		bFadeOut = false;
	}

	// On the same target, move to it.
	if (IsValid(Target) && Target->GetOwner() == Anchor->GetOwner())
	{
		FLatentActionInfo LatentInfo = FLatentActionInfo();;
		LatentInfo.CallbackTarget = this;
		
		UKismetSystemLibrary::MoveComponentTo(RootComponent, Anchor->GetComponentLocation(), FRotator::ZeroRotator, true, true, MoveTime, false, EMoveComponentAction::Type::Move, LatentInfo);
	}
	// Not the same target.
	else
	{
		SetActorLocation(Anchor->GetComponentLocation());
	}

	Target = Anchor;
}

void AGrappleAnchorReticle::Hide()
{
	// Already fading out.
	if (bFadeOut || !Reticle->IsVisible()) { return; }

	if (ReticleFadeValue != 0.0f)
	{
		bFadeIn = false;
		bFadeOut = true;
	}
	else
	{
		Target = nullptr;
		bFadeOut = false;

		Reticle->SetVisibility(false);
	}
}