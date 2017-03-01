#include "Eros.h"

#include "../ErosCharacter.h"
#include "../../InteractableObjects/Magnetic/MagneticCube.h"
#include "../../InteractableObjects/InteractableInterface.h"
#include "DrawDebugHelpers.h"
#include "Prosthetic_MagnetArm.h"
#include "ProstheticSocket.h"

AProsthetic_MagnetArm::AProsthetic_MagnetArm()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AProsthetic_MagnetArm::Tick(float DeltaSeconds)
{
	if (GetProstheticState() == EActionState::AS_Inactive) 
	{
		if (Interactable != nullptr)
		{
			Interactable->StopInteract();
		}
		return;
	}
	Magnetize(GetProstheticState() == EActionState::AS_Primary ? true : false);
}

void AProsthetic_MagnetArm::Magnetize(bool IsPull)
{
	AErosCharacter& Character = GetSocket()->GetCharacter();

	if (Character.GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Falling) return;

	//Start of the ray (Moved slightly forward)
	CurrentMagneticRayStart = Character.GetCameraLocation() + (Character.GetCameraForward() * 100.0f);

	//End of the ray
	CurrentMagneticRayEnd = CurrentMagneticRayStart + (Character.GetCameraForward() * RayLength);

	FCollisionQueryParams CollisionParams = FCollisionQueryParams::DefaultQueryParam;
	CollisionParams.AddIgnoredActor(&Character);

	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(Hit, CurrentMagneticRayStart, CurrentMagneticRayEnd, ECollisionChannel::ECC_GameTraceChannel4, CollisionParams);
	
	DrawDebugLine(GetWorld(), CurrentMagneticRayStart, CurrentMagneticRayEnd, FColor::Green, false, 1.0f, 0, 2.0f);
	
	if (IInteractableInterface* InteractableTemp = Cast<IInteractableInterface>(Hit.GetActor()))
	{
		if (Interactable != InteractableTemp)
		{
			if (Interactable != nullptr)
			{
				Interactable->StopInteract();
			}
			Interactable = InteractableTemp;
		}

		Interactable->Interact(*this);
	}
	else if (Interactable != nullptr)
	{
		Interactable->StopInteract();
		Interactable = nullptr;
	}
}