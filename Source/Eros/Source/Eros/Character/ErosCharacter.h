#pragma once

#include "Prosthetics/ProstheticType.h"
#include "GameFramework/Character.h"
#include "ErosCharacter.generated.h"

class AProsthetic;
class IInteractableInterface;
class UProstheticSocket;
class UPawnNoiseEmitterComponent;
class USoundCue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FProstheticSwapped, AErosCharacter*, PlayerCharacter);

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	CS_OnGround		UMETA(DisplayName = On Ground),
	CS_InAir		UMETA(DisplayName = In Air),
	CS_Hanging		UMETA(DisplayName = Hanging),
	CS_Dropping		UMETA(DisplayName = Dropping),
	CS_Sliding		UMETA(DisplayName = Sliding),
	CS_Climbing		UMETA(DisplayName = Climbing),
	CS_WallRunVert	UMETA(DisplayName = VerticalWallRun),
	CS_WallRunHoriz	UMETA(DisplayName = HorizontalWallRun),
	CS_Crouching	UMETA(DisplayName = Crouching),
	CS_Crawling		UMETA(DisplayName = Crawling)
};

/* Struct used to pass movement information between the character and prosthetics. */
struct FErosCharacterMovement
{
	float JumpZVelocity;
	float AirControl;
	float MaxWalkSpeed;
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

	virtual void BeginPlay() override;

	/* Get the arm socket on the character. */
	UFUNCTION(BlueprintCallable, Category = Prosthetics)
	FORCEINLINE UProstheticSocket* GetArmSocket() { return ArmSocket; }

	/* Get the leg socket on the character. */
	UFUNCTION(BlueprintCallable, Category = Prosthetics)
	FORCEINLINE UProstheticSocket* GetLegSocket() { return LegSocket; }

	/* Get the forward vector of the camera following the player. */
	FORCEINLINE FVector GetCameraForward() const { return FollowCamera->GetForwardVector(); }

	/* Get the up vector of the camera following the player. */
	FORCEINLINE FVector GetCameraUp() const { return FollowCamera->GetUpVector(); }

	/* Get the position of the camera following the player. */
	FORCEINLINE FVector GetCameraLocation() const { return FollowCamera->GetComponentLocation(); }

	/* Make footsteps Audible to the player and the AI. */
	UFUNCTION(BlueprintCallable, Category = Audio)
	void MakeProstheticFootstepNoise();

	UFUNCTION(BlueprintCallable, Category = Audio)
	void MakeRegularFootstepNoise();

	/* Returns true if a prosthetic of a matching type is currently attached, false otherwise. */
	bool HasProstheticAttached(UClass* Class) const;

	/* Returns true if the given prosthetic can be attached to the character, false otherwise. */
	bool HasFreeSocketOfType(EProstheticType Type) const;

	/* Return the loudest volume between the regular leg and the prosthetic leg. */
	float GetMaxVolume();

	/* Search through all the sockets and remove the first one that has a prosthetic of the given type. */
	AProsthetic* DetachProstheticOfType(EProstheticType Type);

protected:

	/* Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/* Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

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
	class UStaticMeshComponent* CrawlingCollider;

	/* State machine for character movement. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Character)
	ECharacterState CurrentState;

	/* Keeps track of character state in order to detect changes between ticks. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TEnumAsByte<ECharacterState> StateLastTick;

	/* Stores the Tag of the type of ledge currently hanging onto */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	FName LedgeGrabTag;

	/* Keeps track of movement mode in order to detect changes between ticks. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TEnumAsByte<EMovementMode> MovementModeLastTick;

	///* Should the Tick keep track of how long crouch has been held. */
	//bool ShouldTimeCrouch;
	///* Keeps track of how long crouch has been held. */
	//float CrouchTimer;
	///* keeps track of if changing from standing to crawling, or crawling to standing */
	//ECharacterState StateWhenCrouchPressed;

	/* Box used to detect if a prosthetic is within picking up range. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Prosthetics)
	UBoxComponent* ProstheticPickupZone;

	virtual void Tick(float DeltaSeconds) override;

	/* Setup the controls for this character. */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/* override ACharacter jump, to incorperate free running logic */
	void Jump() override;

	/* Called via input to crouch */
	void ErosCrouchPressed();
	//void ErosCrouchReleased();

	/* Used to measure how long crouch has been held for and to change state after certain time */
	//void ErosCrouchTimer(float DeltaSeconds);

	/* Called for forwards/backward input. */
	void MoveForward(float Value);

	/* Called for side to side input. */
	void MoveRight(float Value);

	/* Primary action begin. */
	void LeftMouseDown();

	/* Primary action end. */
	void LeftMouseUp();

	/* Secondary action begin. */
	void RightMouseDown();

	/* Secondary action end. */
	void RightMouseUp();

	/*
	* Called via input to turn at a given rate.
	*
	* Rate: This is a normalized rate, i.e. 1.0 means 100% of desired turn rate.
	*/
	void TurnAtRate(float Rate);

	/*
	* Called via input to turn look up/down at a given rate.
	*
	* Rate: This is a normalized rate, i.e. 1.0 means 100% of desired turn rate.
	*/
	void LookUpAtRate(float Rate);

	/* Returns the closests prosthetics of the given type, if one is available. */
	AProsthetic* GetClosestProstheticOfType(EProstheticType Type);

	/* Returns the closest interactable object. */
	IInteractableInterface* GetClosestInteractable();

	/* Removes the current leg prosthetic (if available) and attaches the next closest leg prosthetic (if available). */
	void SwapLegProsthetic();

	/* Removes the current arm prosthetic (if available) and attaches the next closest arm prosthetic (if available). */
	void SwapArmProsthetic();

	/* Check if the character can grab a ledge. */
	void CheckLedgeGrab();

	/* Attempt to attach the given prosthetic to a free socket. */
	bool AttachProsthetic(AProsthetic& Prosthetic);

private:

	/* Socket for an arm prosthetic. */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetics)
	UProstheticSocket* ArmSocket;

	/* Socket for a leg prosthetic. */
	UPROPERTY(EditDefaultsOnly, Category = Prosthetics)
	UProstheticSocket* LegSocket;

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

	/* Interactable objects which are in interact range. */
	TArray<IInteractableInterface*> NearbyInteractables;

	/* Base character movement values. */
	FErosCharacterMovement DefaultCharacterMovement;

	/* World position at which the character started falling. */
	FVector FallStartPosition;

	/* Is the character currently falling? */
	bool bWasFalling;

	/* The left mouse is pressed down... */
	bool bIsLeftMouseDown;

	/* The right mouse is pressed down... */
	bool bIsRightMouseDown;

	/* If the camera is too close to the character, turn the character and prosthetic meshes off. */
	bool bIsCameraTooClose;

	/* Component to play player sounds. */
	UAudioComponent* AudioComponent;

	/* Sound to play when the prosthetic touches the ground. */
	UPROPERTY(EditDefaultsOnly, Category = Audio)
	USoundCue* FootstepSound;

	/* Volume of the default footstep. */
	UPROPERTY(EditDefaultsOnly, Category = Audio)
	float FootstepVolume;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnCollided(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/* Attempts to interact with the closest interactable object. */
	void Interact();

	/* Loops through each prosthetic and applies any modifiers. */
	FErosCharacterMovement GetModifiedCharacterMovement(FErosCharacterMovement DefaultMovement) const;

	/* Apply changes to the character movement component. */
	void UpdateCharacterMovement(FErosCharacterMovement Movement);

	/* Check if the character has taken fall damage. */
	void CheckFallDamage();

	/* Run this upon death to revert to the last checkpoint. */
	void KillPlayer();

	/* Update the current character state and speed. */
	void UpdateCharacterState();

	/* Check if the character mesh should be turned off if the camera is too close. */
	void CheckCameraDistance();
};