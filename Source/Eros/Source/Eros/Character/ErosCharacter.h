#pragma once

#include "Prosthetics/ProstheticType.h"
#include "GameFramework/Character.h"
#include "ErosCharacter.generated.h"

class AInteractableActor;
class AProsthetic;
class AUnattachedProsthetic;
class UProstheticSocket;
class UPawnNoiseEmitterComponent;
class USoundCue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFallDamageChanged, float, FallDamage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FProstheticSwapped, AErosCharacter*, PlayerCharacter, AProsthetic*, NewProsthetic);

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	CS_OnGround		UMETA(DisplayName = On Ground),
	CS_Crouching	UMETA(DisplayName = Crouching),
	CS_Crawling		UMETA(DisplayName = Crawling),
	CS_InAir		UMETA(DisplayName = In Air),

	CS_Hanging		UMETA(DisplayName = Hanging),
	CS_Dropping		UMETA(DisplayName = Dropping),
	CS_Climbing		UMETA(DisplayName = Climbing),
	CS_WallRunVert	UMETA(DisplayName = VerticalWallRun),

	CS_LegOff		UMETA(DisplayName = LegOff),
	CS_LegOn		UMETA(DisplayName = LegOn),
	CS_ArmOff		UMETA(DisplayName = ArmOff),
	CS_ArmOn		UMETA(DisplayName = ArmOn),
	CS_StandUp		UMETA(DisplayName = StandUp),
	CS_StartCrawl	UMETA(DisplayName = StartCrawl),

	CS_Aiming		UMETA(DisplayName = Aiming),
	CS_Grappling	UMETA(DisplayName = Grappling),

	CS_CraneHang	UMETA(DisplayName = CraneHang)
};

/* Struct used to pass movement information between the character and prosthetics. */
USTRUCT()
struct FErosCharacterMovement
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	float JumpZVelocity;

	UPROPERTY(EditAnywhere)
	float AirControl;

	UPROPERTY(EditAnywhere)
	float WalkSpeed;

	UPROPERTY(EditAnywhere)
	float CrouchSpeed;

	UPROPERTY(EditAnywhere)
	float CrawlSpeed;

	UPROPERTY(EditAnywhere)
	FRotator RotationRate;
};

UCLASS(config = Game)
class AErosCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	AErosCharacter();

	UPROPERTY(BlueprintAssignable)
	FProstheticSwapped OnProstheticSwapped;

	UPROPERTY(BlueprintAssignable)
	FOnFallDamageChanged OnFallDamageChanged;

	virtual void BeginPlay() override;

	/* Get the arm socket on the character. */
	UFUNCTION(BlueprintCallable, Category = Prosthetics)
	FORCEINLINE UProstheticSocket* GetArmSocket() { return ArmSocket; }

	/* Get the leg socket on the character. */
	UFUNCTION(BlueprintCallable, Category = Prosthetics)
	FORCEINLINE UProstheticSocket* GetLegSocket() { return LegSocket; }

	UFUNCTION(BlueprintCallable, Category = Prosthetics)
	FORCEINLINE bool GetShowTutorial() const { return true; }

	/* Play a voiceline. */
	UFUNCTION(BlueprintCallable, Category = Voice)
	void Say(USoundCue* Voiceline);

	/* Get the forward vector of the camera following the player. */
	FORCEINLINE FVector GetCameraForward() const { return FollowCamera->GetForwardVector(); }

	/* Get the up vector of the camera following the player. */
	FORCEINLINE FVector GetCameraUp() const { return FollowCamera->GetUpVector(); }

	/* Get the position of the camera following the player. */
	FORCEINLINE FVector GetCameraLocation() const { return FollowCamera->GetComponentLocation(); }

	/* Get the current Animation State fcuk you*/
	FORCEINLINE ECharacterState GetState() const { return CurrentState; }

	FORCEINLINE AUnattachedProsthetic* GetDroppedArmProsthetic() { return DroppedArmProsthetic; }
	FORCEINLINE AUnattachedProsthetic* GetDroppedLegProsthetic() { return DroppedLegProsthetic; }

	/* Make footsteps Audible to the player and the AI. */
	UFUNCTION(BlueprintCallable, Category = Audio)
	void MakeProstheticFootstepNoise();

	UFUNCTION(BlueprintCallable, Category = Audio)
	void MakeRegularFootstepNoise();

	UFUNCTION(BlueprintCallable, Category = State)
	void ChangeState(ECharacterState NewState);

	/* Returns true if a prosthetic of a matching type is currently attached, false otherwise. */
	bool HasProstheticAttached(UClass const* Class) const;

	/* Returns true if the character has a prosthetic of the given type attached, false otherwise. */
	bool HasProstheticOfType(EProstheticType Type) const;

	/* Return the loudest volume between the regular leg and the prosthetic leg. */
	float GetMaxVolume() const;

	/* Search through all the sockets and remove the first one that has a prosthetic of the given type. */
	AProsthetic* DetachProstheticOfType(EProstheticType Type);

	/* Override ACharacter jump, to incorperate free running logic. */
	void Jump() override;

	/* Override ACharacter crouch to handle different states. */
	void Crouch(bool bClientSimulation = false) override;
	
	/* Called for forwards/backward input. */
	void MoveForward(float Value);

	/* Called for side to side input. */
	void MoveRight(float Value);

	/* Primary action begin. */
	void PrimaryActionBegin();

	/* Primary action end. */
	void PrimaryActionEnd();

	/* Secondary action begin. */
	void SecondaryActionBegin();

	/* Secondary action end. */
	void SecondaryActionEnd();

	/* Removes the current leg prosthetic (if available) and attaches the next closest leg prosthetic (if available). */
	void SwapLegProsthetic();
	/* Attaches the old leg prosthetic to the hand bone */
	UFUNCTION(BlueprintCallable, Category = "SwapLeg")
	void DetachLegProsthetic();
	/* Drops the old leg prosthetic on the floor */
	UFUNCTION(BlueprintCallable, Category = "SwapLeg")
	void LegProstheticDetached();
	/* Attaches new leg prosthetic to hand bone */
	UFUNCTION(BlueprintCallable, Category = "SwapLeg")
	void AttachLegProsthetic();
	/* Attaches new leg prosthetic to leg socket */
	UFUNCTION(BlueprintCallable, Category = "SwapLeg")
	void LegProstheticAttached();

	/* Removes the current arm prosthetic (if available) and attaches the next closest arm prosthetic (if available). */
	void SwapArmProsthetic();
	/* Attaches the old arm prosthetic to the hand bone */
	UFUNCTION(BlueprintCallable, Category = "SwapArm")
	void DetachArmProsthetic();
	/* Drops the old arm prosthetic on the floor */
	UFUNCTION(BlueprintCallable, Category = "SwapArm")
	void ArmProstheticDetached();
	/* Attaches new arm prosthetic to hand bone */
	UFUNCTION(BlueprintCallable, Category = "SwapArm")
	void AttachArmProsthetic();
	/* Attaches new arm prosthetic to leg socket */
	UFUNCTION(BlueprintCallable, Category = "SwapArm")
	void ArmProstheticAttached();

	/* Returns the first prosthetic of ProstheticType that the character is holding (in their hands), or a nullptr if not. */
	AActor* GetHeldProstheticOfType(EProstheticType ProstheticType);

	bool CanUseProsthetic() const;

	/* Attempts to interact with the closest interactable object. */
	void Interact();

	/* Attempt to attach the given prosthetic to a free socket. */
	bool AttachProsthetic(AProsthetic& Prosthetic);

	/* Run this upon death to revert to the last checkpoint. */
	void KillPlayer();

	/* When destroying the character the prosthetics must be removed and destroyed. */
	UFUNCTION()
	virtual void Destroyed() override;

	/* Breaks the players arm and leg prosthetics, updating their state to crawling. */
	void FuckIFellAndBrokeEverything();

	// Resets the distance the player has fallen, used to negate fall damage.
	void ResetFallDistance();

	void ResetForNewGame();

protected:

	/* Fall distance value at which the character will die. Must be a negative value. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fall Damage")
	float MaxFallDistance;

	/* Fall distance value at which the character will start taking fall damage. Must be a negative value. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fall Damage")
	float MinFallDistance;

	/* Camera boom positioning the camera behind the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	USpringArmComponent* CameraBoom;

	/* Follow camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* FollowCamera;

	/* Capsule Collider for crawling movement*/
	UPROPERTY(EditDefaultsOnly, Category = CrawlingCollider)
	UStaticMeshComponent* CrawlingCollider;

	/* State machine for character movement. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Character)
	ECharacterState CurrentState;

	/* Stores the Tag of the type of ledge currently hanging onto */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	FName LedgeGrabTag;

	/* Box used to detect if a prosthetic is within picking up range. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Prosthetics)
	USphereComponent* InteractZone;

	virtual void Tick(float DeltaSeconds) override;
	
	/* Returns the closests prosthetics of the given type, if one is available. */
	AUnattachedProsthetic* GetClosestProstheticOfType(EProstheticType Type);

	/* Returns the closest interactable object. */
	AInteractableActor* GetClosestInteractable() const;

private:
	
	UPROPERTY(EditDefaultsOnly, Category = Voice)
	UAudioComponent* CharacterVoice;

	/* Socket for an arm prosthetic. */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetics)
	UProstheticSocket* ArmSocket;

	/* Socket for a leg prosthetic. */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetics)
	UProstheticSocket* LegSocket;

	/* Stores the new leg prosthetic to attach, before the old one is removed */
	AUnattachedProsthetic* NewLegProstheticToAttach;
	/* Stores the new arm prosthetic to attach, before the old one is removed */
	AUnattachedProsthetic* NewArmProstheticToAttach;

	/* Component to create noise events the AI can react to. */
	UPROPERTY(EditDefaultsOnly, Category = AI)
	UPawnNoiseEmitterComponent* LocalNoiseEmitter;

	UPROPERTY(EditDefaultsOnly, Category = "Basic Movement")
	float BaseJumpVelocity;

	/* Arm to spawn and attach to the character. */
	UPROPERTY(EditDefaultsOnly, Category = "Default Limbs")
	TSubclassOf<AProsthetic> DefaultArm;

	/* Leg to spawn and attach to the character. */
	UPROPERTY(EditDefaultsOnly, Category = "Default Limbs")
	TSubclassOf<AProsthetic> DefaultLeg;

	/* Base character movement values. */
	UPROPERTY(EditDefaultsOnly, Category = "Eros Movement")
	FErosCharacterMovement DefaultCharacterMovement;

	/* Sound to play when the prosthetic touches the ground. */
	UPROPERTY(EditDefaultsOnly, Category = Audio)
	USoundCue* FootstepSound;

	/* Volume of the default footstep. */
	UPROPERTY(EditDefaultsOnly, Category = Audio)
	float FootstepVolume;

	/* Fall damage percent recovered per second. */
	UPROPERTY(EditDefaultsOnly, Category = "Fall Damage", meta = (ClampMin = "0.01", ClampMax = "1.0", UIMin = "0.01", UIMax = "1.0"))
	float FallDamageRecovery;

	/* Fall damage taken by the character, in range (0.0 - 1.0) */
	float FallDamagePercent;

	/* Interactable objects which are in interact range. */
	TArray<AInteractableActor*> NearbyInteractables;

	/* World position at which the character started falling. */
	FVector FallStartPosition;

	/* Is the character currently falling? */
	bool bWasFalling;

	/* If the camera is too close to the character, turn the character and prosthetic meshes off. */
	bool bIsCameraTooClose;

	/* Component to play player sounds. */
	UAudioComponent* AudioComponent;

	/* Current closests interactables. */
	AInteractableActor* NearestLeg;
	AInteractableActor* NearestArm;
	AInteractableActor* NearestButton;

	/* The last interactable objects to be highlighted. */
	AInteractableActor* PreviousNearestLeg;
	AInteractableActor* PreviousNearestArm;
	AInteractableActor* PreviousNearestButton;

	/* Last prosthetics to be dropped. */
	AUnattachedProsthetic* DroppedArmProsthetic;
	AUnattachedProsthetic* DroppedLegProsthetic;

	AUnattachedProsthetic* ArmProstheticToRemove;
	AUnattachedProsthetic* LegProstheticToRemove;
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnCapsuleOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnCollided(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/* Check if the character can grab a ledge. */
	void CheckLedgeGrab();

	/* Loops through each prosthetic and applies any modifiers. */
	FErosCharacterMovement GetModifiedCharacterMovement(FErosCharacterMovement DefaultMovement) const;

	/* Apply changes to the character movement component. */
	void UpdateCharacterMovement(FErosCharacterMovement Movement);

	/* Check if the character has taken fall damage. */
	void CheckFallDamage();

	/* Applies recovery to the players fall damage. */
	void UpdateFallDamage(float DeltaSeconds);

	/* Check if the character mesh should be turned off if the camera is too close. */
	void CheckCameraDistance();

	/* Enter a new character state. */
	void EnterState(ECharacterState State);

	/* Exit a state (should be the current state). */
	void ExitState(ECharacterState State);

	/* Update the current state. Call every frame. */
	void TickState(float DeltaSeconds);

	/* Locate the nearest leg, arm, and interactable object within NearbyInteractables and set their UI to visible, setting the UI of all others to invisible. */
	void UpdateInteractableHighlights();
};