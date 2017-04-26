#pragma once

#include "../../InteractableObjects/InteractableActor.h"
#include "ProstheticType.h"

#include "Prosthetic.generated.h"

class UProstheticSocket;
class USoundCue;
class USlateBrushAsset;
class UUserWidget;

struct FErosCharacterMovement;

UENUM()
enum class EActionState : uint8
{
	AS_Inactive,
	AS_Primary,
	AS_Secondary
};

/* The most basic prosthetic; has no abilities or modifiers for the character. */
UCLASS()
class EROS_API AProsthetic : public AInteractableActor
{
	GENERATED_BODY()

	/* The socket controls whether a prosthetic can be attached or not. */
	friend UProstheticSocket;

public:

	AProsthetic();

	UFUNCTION(BlueprintCallable, Category = Prosthetic)
	FORCEINLINE TSubclassOf<UUserWidget> GetHelpWidget() { return HelpWidget; }
	
	/* Get the prosthetic type. */
	FORCEINLINE EProstheticType GetType() const { return ProstheticType; }

	/* Get the prosthetic reference. */
	FORCEINLINE TSubclassOf<AUnattachedProsthetic> GetUnattachedProsthetic() const { return UnattachedProsthetic; }

	/* Get the socket this prosthetic is attached to. May be null! */
	FORCEINLINE UProstheticSocket* GetSocket() { return AttachedSocket; }

	/* Is this prosthetic attached to a socket? */
	FORCEINLINE bool IsAttachedToSocket() const { return AttachedSocket != nullptr; }

	/* How much noise the prosthetic makes. */
	FORCEINLINE float GetSoundVolume() const { return SoundVolume; }

	/* Get the action state of the prosthetic. */
	FORCEINLINE EActionState GetProstheticState() const { return ActionState; }

	/* Get the Prosthetic's Mesh */
	FORCEINLINE USkeletalMeshComponent* GetMesh() { return ProstheticMesh; }
	
	/* Get the prosthetic texture to use in the HUD. */
	UFUNCTION(BlueprintCallable, Category = UI)
	FORCEINLINE USlateBrushAsset* GetUiComponent() const { return UiSlateBrush; }

	virtual void BeginPlay() override;

	virtual void PlayFootstepSound() const;

	virtual void SetMeshVisibility(bool bShow);

	/*
	* Apply any modifiers to the characters movement.
	*
	* Returns the modified character movement.
	*/
	virtual FErosCharacterMovement ApplyModifiers(FErosCharacterMovement CharacterMovement) const;

	/* Performs the primary action of a prosthetic. */
	virtual void PrimaryActionBegin();

	/* Ends the primary action of a prosthetic. */
	virtual void PrimaryActionEnd();

	/* Performs the secondary action of a prosthetic. */
	virtual void SecondaryActionBegin();

	/* Ends the secondary action of a prosthetic. */
	virtual void SecondaryActionEnd();

protected:

	/* Called when the prosthetic is attached to a new socket. */
	virtual void OnAttached();

	/* Called when the prosthetic is detached from the current socket. */
	virtual void OnDetached();

	/* Action state of the prosthetic.... */
	EActionState ActionState;
private:

	/* The type of this particular prosthetic. */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetic)
	EProstheticType ProstheticType;

	/* The unattached version of this prosthetic. */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetic)
	TSubclassOf<AUnattachedProsthetic> UnattachedProsthetic;

	/* The prosthetic mesh... */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetic)
	USkeletalMeshComponent* ProstheticMesh;

	/* Collider used by ErosCharacter to detect the prosthetic is nearby and thus can be picked up. */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetic)
	USceneComponent* PickupCollider;

	/* Sound to play when the prosthetic touches the ground. */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetic)
	USoundCue* FootstepSound;

	/* The volume of the noise created by the prosthetic. */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetic, meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float SoundVolume;

	/* Texture to display on the game HUD when this prosthetic is attached. */
	UPROPERTY(EditDefaultsOnly, Category = UI)
	USlateBrushAsset* UiSlateBrush;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UUserWidget> HelpWidget;

	/* Component to play prosthetic sounds. */
	UAudioComponent* AudioComponent;

	/* The socket the prosthetic is currently attached to. */
	UProstheticSocket* AttachedSocket;

	/*
	* Attach this prosthetic to a socket. Can only be called by UProstheticSocket.
	*
	* Returns: True if the prosthetic was successfully attached, false otherwise.
	*/
	bool AttachToSocket(UProstheticSocket& Socket);

	/* Detach this prosthetic from the current socket. Can only be called by UProstheticSocket. */
	void DetachFromSocket();
};

UCLASS()
class EROS_API AProsthetic_BasicLeg : public AProsthetic
{
	GENERATED_BODY()
};

UCLASS()
class EROS_API AProsthetic_BasicArm : public AProsthetic
{
	GENERATED_BODY()
};