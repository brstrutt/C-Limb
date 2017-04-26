#include "Eros.h"
#include "Character/ErosCharacter.h"
#include "../Character/Prosthetics/UnattachedProsthetic.h"
#include "Managers/GameCuesManager.h"
#include "ProstheticCabinet.h"
#include "Runtime/SlateCore/Public/Layout/SlateRect.h"
#include "Runtime/SlateCore/Public/Layout/ArrangedWidget.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"
#include "Switch/Switch.h"

#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>

AProstheticCabinet::AProstheticCabinet()
{
	if (RootComponent == nullptr)
	{
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	}

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	bActive = true;
}

void AProstheticCabinet::BeginPlay()
{
	Super::BeginPlay();

	ProstheticInstance = GetWorld()->SpawnActor<AUnattachedProsthetic>(ProstheticToSpawn.GetDefaultObject()->GetClass());

	// BUG FIX: Begin play doesn't seem to get called until the next frame, but we really need the logic playing now.
	ProstheticInstance->BeginPlay();

	ProstheticInstance->CanSimulatePhysics(false);
	ProstheticInstance->SetActorEnableCollision(false);
	ProstheticInstance->SetInteractable(false);

	if (ProstheticInstance->GetType() == EProstheticType::PT_Arm)
	{
		ProstheticInstance->AttachToComponent(Mesh, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false), FName("ProstheticArmSocket"));
	}
	else
	{
		ProstheticInstance->AttachToComponent(Mesh, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false), FName("ProstheticLegSocket"));
	}
	
	// Delegate to enable a switch, if one is present.
	FScriptDelegate AttachedDel;
	AttachedDel.BindUFunction(this, FName("ProstheticAttached"));
	ProstheticInstance->OnProstheticAttached.Add(AttachedDel);

	SetHighlightVisible(bActive);
}

bool AProstheticCabinet::Interact(AErosCharacter& Character)
{
	// only animate once, when closed
	if (!bIsOpen && bActive)
	{
		bIsOpen = true;

		// play animation, but not as a loop
		Mesh->Play(false);
		SetInteractable(false);
		SetUIVisible(false);

		ProstheticInstance->SetActorEnableCollision(true);
		ProstheticInstance->SetInteractable(true);

		return true;
	}
	return false;
}

void AProstheticCabinet::SetUIVisible(bool bShowUi)
{
	Super::SetUIVisible(bShowUi && !bIsOpen && bActive);
}

void AProstheticCabinet::SetHighlightVisible(bool bShowHighlight)
{
	Mesh->SetRenderCustomDepth(bShowHighlight);
}

void AProstheticCabinet::SetInteractable(bool NewActive)
{
	Super::SetInteractable(NewActive);

	bActive = NewActive;
	SetHighlightVisible(NewActive);
}

void AProstheticCabinet::ProstheticAttached()
{
	if (SwitchToActivate != nullptr)
	{
		SwitchToActivate->SetInteractable(true);
	}

	if (GameCue != nullptr)
	{
		AErosManager::GetManager<AGameCuesManager>(GetWorld())->PlayCue(*GameCue.GetDefaultObject());
	}

	AErosCharacter* Character = Cast<AErosCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());

	if (bIsTutorial && Character)
	{
		if (ProstheticToSpawn.GetDefaultObject()->GetType() == EProstheticType::PT_Arm)
		{
			if (Character->GetDroppedArmProsthetic() != nullptr)
			{
				Character->GetDroppedArmProsthetic()->SetInteractable(false);
				Character->GetDroppedArmProsthetic()->SetUIVisible(false);
				Character->GetDroppedArmProsthetic()->SetActorEnableCollision(false);
				Character->GetDroppedArmProsthetic()->CanSimulatePhysics(false);
			}
		}
		else
		{
			if (Character->GetDroppedLegProsthetic() != nullptr)
			{
				Character->GetDroppedLegProsthetic()->SetInteractable(false);
				Character->GetDroppedLegProsthetic()->SetUIVisible(false);
				Character->GetDroppedLegProsthetic()->SetActorEnableCollision(false);
				Character->GetDroppedLegProsthetic()->CanSimulatePhysics(false);
			}
		}
	}
}