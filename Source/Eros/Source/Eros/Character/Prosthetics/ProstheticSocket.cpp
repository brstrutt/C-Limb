#include "Eros.h"

#include "../ErosCharacter.h"
#include "Prosthetic.h"
#include "ProstheticSocket.h"

UProstheticSocket::UProstheticSocket()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UProstheticSocket::BeginPlay()
{
	Super::BeginPlay();

	CharacterAttachedTo = Cast<AErosCharacter>(GetAttachmentRootActor());
}

bool UProstheticSocket::TryAttachProsthetic(AProsthetic& Prosthetic)
{
	if (HasProsthetic()) { return false; }

	bool bSuccess = Prosthetic.GetType() == ProstheticType && Prosthetic.AttachToSocket(*this);

	if (bSuccess)
	{
		AttachedProsthetic = &Prosthetic;

		OnProstheticUpdated.Broadcast(&Prosthetic);
	}

	return bSuccess;
}

AProsthetic* UProstheticSocket::DetachProsthetic()
{
	if (!HasProsthetic()) { return nullptr; }

	// Detach prosthetic.
	AProsthetic* DetachedProsthetic = AttachedProsthetic;
	DetachedProsthetic->DetachFromSocket();

	// Ensure attached prosthetic is set to null.
	AttachedProsthetic = nullptr;

	OnProstheticUpdated.Broadcast(nullptr);

	return DetachedProsthetic;
}