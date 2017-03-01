#include "Eros.h"

#include "../ErosCharacter.h"
#include "Prosthetic.h"
#include "ProstheticSocket.h"
#include "Sound/SoundCue.h"

AProsthetic::AProsthetic()
{
	PrimaryActorTick.bCanEverTick = false;
	
	RootComponent = ProstheticMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Prosthetic"));

	PickupCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("PickupCollider"));
	PickupCollider->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
}

void AProsthetic::PlayFootstepSound() const
{
	if (FootstepSound == nullptr) { return; }

	AudioComponent->SetVolumeMultiplier(SoundVolume);
	AudioComponent->SetSound(FootstepSound);
	AudioComponent->Play();
}

bool AProsthetic::AttachToSocket(UProstheticSocket& Socket)
{
	// Already attached to a socket.
	if (IsAttachedToSocket()) { return false; }

	// MUST be below the skeletal mesh component in the heirarchy.
	AttachToComponent(&Socket, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
	// Call this so the prosthetic animates too.
	ProstheticMesh->SetMasterPoseComponent(Cast<ACharacter>(Socket.GetAttachmentRootActor())->GetMesh());

	// Ensure it is position correctly.
	RootComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	RootComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	
	AttachedSocket = &Socket;

	OnAttached();
	
	return true;
}

void AProsthetic::DetachFromSocket()
{
	if (IsAttachedToSocket())
	{
		DetachRootComponentFromParent(true);
		ProstheticMesh->SetMasterPoseComponent(nullptr);

		AttachedSocket = nullptr;

		OnDetached();
	}
}

FErosCharacterMovement AProsthetic::ApplyModifiers(FErosCharacterMovement CharacterMovement) const
{
	return CharacterMovement;
}

void AProsthetic::PrimaryActionBegin()
{
	if (ActionState == EActionState::AS_Inactive)
	{
		ActionState = EActionState::AS_Primary;
	}
}

void AProsthetic::PrimaryActionEnd()
{
	if (ActionState == EActionState::AS_Primary)
	{
		ActionState = EActionState::AS_Inactive;
	}
}

void AProsthetic::SecondaryActionBegin()
{
	if (ActionState == EActionState::AS_Inactive)
	{
		ActionState = EActionState::AS_Secondary;
	}
}

void AProsthetic::SecondaryActionEnd()
{
	if (ActionState == EActionState::AS_Secondary)
	{
		ActionState = EActionState::AS_Inactive;
	}
}