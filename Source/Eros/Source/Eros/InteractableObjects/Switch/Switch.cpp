#include "Eros.h"

#include "Managers/GameCuesManager.h"
#include "Sound/SoundCue.h"
#include "Switch.h"
#include "Runtime/SlateCore/Public/Layout/SlateRect.h"
#include "Runtime/SlateCore/Public/Layout/ArrangedWidget.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"

ASwitch::ASwitch()
{
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	ProximityTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Proximity Box"));
	ProximityTrigger->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
	ProximityTrigger->SetBoundsScale(3.051758f);
}

void ASwitch::BeginPlay()
{
	Super::BeginPlay();

	SetInteractable(bStartEnabled);
}

void ASwitch::SetUIVisible(bool bShowUi)
{
	if (!(bSingleActivation && bHasActivated))
	{
		Super::SetUIVisible(bShowUi);
	}
}

void ASwitch::SetHighlightVisible(bool bShowHighlight)
{
	Mesh->SetRenderCustomDepth(bShowHighlight);
}

void ASwitch::SetInteractable(bool bActive)
{
	Super::SetInteractable(bActive);
	
	// Toggle the Proximity trigger's collision
	ProximityTrigger->SetCollisionEnabled( (bActive) ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision );
}

bool ASwitch::Interact(AErosCharacter& Character)
{
	if (!bSingleActivation || !bHasActivated)
	{
		UE_LOG(LogTemp, Warning, TEXT("INTERACT"));

		if (bHasActivated)
		{
			Swap();
		}
		else if (GameCue != nullptr)
		{
			// Play the game cue just once.
			AErosManager::GetManager<AGameCuesManager>(GetWorld())->PlayCue(*GameCue.GetDefaultObject());
		}

		if (bSingleActivation)
		{
			SetInteractable(false);
			SetUIVisible(false);
		}

		bHasActivated = true;
		
		if (InteractSound != nullptr)
		{
			AudioComponent->SetSound(InteractSound);
			AudioComponent->Play();
		}		

		Activate();

		return true;
	}

	return false;
}