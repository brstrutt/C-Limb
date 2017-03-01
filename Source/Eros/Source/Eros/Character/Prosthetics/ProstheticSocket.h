#pragma once

#include "Prosthetic.h"
#include "ProstheticSocket.generated.h"

class AErosCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FProstheticUpdated, AProsthetic*, Prosthetic);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EROS_API UProstheticSocket : public USceneComponent
{
	GENERATED_BODY()

public:	
	
	UProstheticSocket();

	/* Even called when a prosthetic is attached or detached. */
	UPROPERTY(BlueprintAssignable)
	FProstheticUpdated OnProstheticUpdated;

	/* The type of prosthetic this socket supports. */
	FORCEINLINE EProstheticType GetProstheticType() const { return ProstheticType; }

	/* The attached prosthetic. May be null. */
	UFUNCTION(BlueprintCallable, Category = Prosthetic)
	FORCEINLINE AProsthetic* GetProsthetic() { return AttachedProsthetic; }

	/* Is there a prosthetic attached to the socket? */
	UFUNCTION(BlueprintCallable, Category = Prosthetic)
	FORCEINLINE bool HasProsthetic() const { return AttachedProsthetic != nullptr; }

	/* Get the character this socket is attached to. */
	FORCEINLINE AErosCharacter& GetCharacter() { return *CharacterAttachedTo; }

	virtual void BeginPlay() override;
	
	/*
	 * Attach a prosthetic to this socket.
	 *
	 * Returns: True if the prosthetic was attached, false otherwise.
	 */
	bool TryAttachProsthetic(AProsthetic& Prosthetic);

	/* Detaches and returns the attached socket. */
	AProsthetic* DetachProsthetic();

private:

	/* The support prosthetic type. */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetic)
	EProstheticType ProstheticType;
	
	/* The attached prosthetic. */
	AProsthetic* AttachedProsthetic;

	/* The character this socket is a part of. */
	AErosCharacter* CharacterAttachedTo;
};
