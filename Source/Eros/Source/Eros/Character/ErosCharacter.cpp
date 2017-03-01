#include "Eros.h"

#include "../AI/DroneAICharacter.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "DrawDebugHelpers.h"
#include "ErosCharacter.h"
#include "Prosthetics/ProstheticSocket.h"
#include "Prosthetics/Prosthetic.h"
#include "Prosthetics/Prosthetic_MagnetArm.h"
#include "Prosthetics/Prosthetic_GrappleArm.h"
#include "Sound/SoundCue.h"

AErosCharacter::AErosCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Set up Crawling Capsule
	CrawlingCollider = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Capsule"));
	CrawlingCollider->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	ArmSocket = CreateDefaultSubobject<UProstheticSocket>(TEXT("ArmSocket"));
	ArmSocket->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	LegSocket = CreateDefaultSubobject<UProstheticSocket>(TEXT("LegSocket"));
	LegSocket->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	// Prosthetics pickup zone.
	ProstheticPickupZone = CreateDefaultSubobject<UBoxComponent>(TEXT("ProstheticsPickupZone"));
	ProstheticPickupZone->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	// Delegate for moving into prosthetic pickup range.
	FScriptDelegate ProstheticInRangeDel;
	ProstheticInRangeDel.BindUFunction(this, FName("OnOverlapBegin"));
	ProstheticPickupZone->OnComponentBeginOverlap.Add(ProstheticInRangeDel);

	// Delegate for moving out of prosthetic pickup range.
	FScriptDelegate ProstheticOutOfRangeDel;
	ProstheticOutOfRangeDel.BindUFunction(this, FName("OnOverlapEnd"));
	ProstheticPickupZone->OnComponentEndOverlap.Add(ProstheticOutOfRangeDel);

	// PawnNoiseEmitter is required to generate Noise events that can be detected by the AI
	LocalNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("LocalNoiseEmitterComp"));

	// By default crouch is not held
	//ShouldTimeCrouch = false;

	// Create audio component
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	// Default footstep volume.
	FootstepVolume = 0.3f;
	

	// Delegate for collisions with the core body of the character
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AErosCharacter::OnCollided);
}

void AErosCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Store all character movement variables that are subject to change here!
	DefaultCharacterMovement.AirControl = GetCharacterMovement()->AirControl;
	DefaultCharacterMovement.JumpZVelocity = BaseJumpVelocity;
	DefaultCharacterMovement.RotationRate = GetCharacterMovement()->RotationRate;
	DefaultCharacterMovement.MaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	// Spawn and attach default limbs.

	AttachProsthetic(*GetWorld()->SpawnActor<AProsthetic>(DefaultArm.GetDefaultObject()->GetClass()));
	AttachProsthetic(*GetWorld()->SpawnActor<AProsthetic>(DefaultLeg.GetDefaultObject()->GetClass()));
}

void AErosCharacter::MakeProstheticFootstepNoise()
{
	if (LegSocket->HasProsthetic())
	{
		// Play the audio clip
		LegSocket->GetProsthetic()->PlayFootstepSound();

		LocalNoiseEmitter->MakeNoise(this, LegSocket->GetProsthetic()->GetSoundVolume(), GetActorLocation());
	}
}

void AErosCharacter::MakeRegularFootstepNoise()
{
	if (FootstepSound == nullptr) { return; }

	AudioComponent->SetVolumeMultiplier(FootstepVolume);
	AudioComponent->SetSound(FootstepSound);
	AudioComponent->Play();

	LocalNoiseEmitter->MakeNoise(this, FootstepVolume, GetActorLocation());
}

float AErosCharacter::GetMaxVolume()
{
	if (LegSocket->HasProsthetic()) return LegSocket->GetProsthetic()->GetSoundVolume();

	return FootstepVolume;
}

void AErosCharacter::Tick(float DeltaSeconds)
{
	UpdateCharacterState();
	CheckLedgeGrab();
	CheckFallDamage();
	CheckCameraDistance();

	MovementModeLastTick = GetCharacterMovement()->MovementMode;
	StateLastTick = CurrentState;

	//if (ShouldTimeCrouch) { ErosCrouchTimer(DeltaSeconds); }
}

void AErosCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AErosCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAxis("MoveForward", this, &AErosCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AErosCharacter::MoveRight);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AErosCharacter::ErosCrouchPressed);
	//PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AErosCharacter::ErosCrouchReleased);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AErosCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AErosCharacter::LookUpAtRate);

	// Controls for picking up/dropping prosthetics.
	PlayerInputComponent->BindAction("SwapLegProsthetic", IE_Pressed, this, &AErosCharacter::SwapLegProsthetic);
	PlayerInputComponent->BindAction("SwapArmProsthetic", IE_Pressed, this, &AErosCharacter::SwapArmProsthetic);
	
	// Controls for Left and right click.
	PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, this, &AErosCharacter::LeftMouseDown);
	PlayerInputComponent->BindAction("SecondaryAction", IE_Pressed, this, &AErosCharacter::RightMouseDown);
	PlayerInputComponent->BindAction("PrimaryAction", IE_Released, this, &AErosCharacter::LeftMouseUp);
	PlayerInputComponent->BindAction("SecondaryAction", IE_Released, this, &AErosCharacter::RightMouseUp);

	// Interacting with interactable objects.
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AErosCharacter::Interact);
}

void AErosCharacter::Jump()
{
	switch (CurrentState)
	{
	case ECharacterState::CS_Hanging:
		if (HasProstheticAttached(AProsthetic_BasicArm::StaticClass())|| HasProstheticAttached(AProsthetic_MagnetArm::StaticClass()) || HasProstheticAttached(AProsthetic_GrappleArm::StaticClass()))
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Flying);
			CurrentState = ECharacterState::CS_Climbing;
			UE_LOG(LogTemp, Warning, TEXT("state changed to climbing"));
		}
		break;
	case ECharacterState::CS_Crawling:
		// cant jump while crawling
		break;
	default:
		bPressedJump = true;
		JumpKeyHoldTime = 0.0f;
		break;
	}
}

void AErosCharacter::ErosCrouchPressed()
{
	//StateWhenCrouchPressed = CurrentState;

	switch (CurrentState)
	{
	case ECharacterState::CS_Hanging:
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		CurrentState = ECharacterState::CS_Dropping;
		UE_LOG(LogTemp, Warning, TEXT("State Changed to Dropping"));
		break;

	case ECharacterState::CS_OnGround:
		CurrentState = ECharacterState::CS_Crouching;
		UE_LOG(LogTemp, Warning, TEXT("State Changed to Crouching"));
		break;

	case ECharacterState::CS_Crouching:
		CurrentState = ECharacterState::CS_OnGround;
		UE_LOG(LogTemp, Warning, TEXT("State Changed to OnGround"));
		break;

	/*case ECharacterState::CS_Crawling:
		CurrentState = ECharacterState::CS_Crouching;
		UE_LOG(LogTemp, Warning, TEXT("State Changed to Crouching"));
		break;*/

	default:
		break;
	}

	//CrouchTimer = 0;
	//ShouldTimeCrouch = true;
}
//void AErosCharacter::ErosCrouchReleased()
//{
//	if (CurrentState == ECharacterState::CS_Crouching)
//	{
//		if (CrouchTimer <= 0.5f)
//		{
//			//tapped
//			if (StateWhenCrouchPressed == ECharacterState::CS_Crouching)
//			{
//				CurrentState = ECharacterState::CS_OnGround;
//				UE_LOG(LogTemp, Warning, TEXT("State Changed to OnGround"));
//			}
//		}
//		else
//		{
//			// held, should be dealt with
//		}
//	}
//
//	CrouchTimer = 0;
//	ShouldTimeCrouch = false;
//}
//void AErosCharacter::ErosCrouchTimer(float DeltaSeconds)
//{
//	CrouchTimer+=DeltaSeconds;
//
//	if (CurrentState == ECharacterState::CS_Crouching && CrouchTimer > 0.5f)
//	{
//		switch (StateWhenCrouchPressed)
//		{
//		case ECharacterState::CS_OnGround:
//		case ECharacterState::CS_Crouching:
//			CurrentState = ECharacterState::CS_Crawling;
//			UE_LOG(LogTemp, Warning, TEXT("State Changed to Crawling"));
//			break;
//		case ECharacterState::CS_Crawling:
//			CurrentState = ECharacterState::CS_OnGround;
//			UE_LOG(LogTemp, Warning, TEXT("State Changed to OnGround"));
//			break;
//		}
//	}
//}

void AErosCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// Apply prosthetic modifiers.
		UpdateCharacterMovement(GetModifiedCharacterMovement(DefaultCharacterMovement));

		switch (CurrentState)
		{
		case ECharacterState::CS_Hanging:
			// Character should not be able to move if hanging from a ledge, atm
			break;
		default:
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, Value);
		}
	}
}

void AErosCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// Apply prosthetic modifiers.
		UpdateCharacterMovement(GetModifiedCharacterMovement(DefaultCharacterMovement));

		switch (CurrentState)
		{
		case ECharacterState::CS_Hanging:
			// Character should not be able to move if hanging from a ledge
			break;
		default:
			// find out which way is right
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
			// get right vector 
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			// add movement in that direction
			AddMovementInput(Direction, Value);
		}
	}
}

void AErosCharacter::LeftMouseDown()
{
	// Only arm prosthetics have a primary action.
	if (ArmSocket->HasProsthetic())
	{
		ArmSocket->GetProsthetic()->PrimaryActionBegin();
	}

}

void AErosCharacter::RightMouseDown()
{
	// Only arm prosthetics have a secondary action.
	if (ArmSocket->HasProsthetic())
	{
		ArmSocket->GetProsthetic()->SecondaryActionBegin();
	}

}

void AErosCharacter::LeftMouseUp()
{
	if (ArmSocket->HasProsthetic())
	{
		ArmSocket->GetProsthetic()->PrimaryActionEnd();
	}

}

void AErosCharacter::RightMouseUp()
{
	if (ArmSocket->HasProsthetic())
	{
		ArmSocket->GetProsthetic()->SecondaryActionEnd();
	}
}

void AErosCharacter::TurnAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AErosCharacter::LookUpAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AErosCharacter::HasProstheticAttached(UClass* Class) const
{
	return ((ArmSocket->HasProsthetic() && ArmSocket->GetProsthetic()->IsA(Class)) || (LegSocket->HasProsthetic() && LegSocket->GetProsthetic()->IsA(Class)));
}

bool AErosCharacter::HasFreeSocketOfType(EProstheticType Type) const
{
	return ((ArmSocket->GetProstheticType() == Type && !ArmSocket->HasProsthetic()) || (LegSocket->GetProstheticType() == Type && !LegSocket->HasProsthetic()));
}

AProsthetic* AErosCharacter::DetachProstheticOfType(EProstheticType Type)
{
	AProsthetic* Prosthetic = nullptr;

	if (CurrentState == ECharacterState::CS_OnGround)
	{
		if (ArmSocket->GetProstheticType() == Type)
		{
			Prosthetic = ArmSocket->DetachProsthetic();
		}
		else if (LegSocket->GetProstheticType() == Type)
		{
			Prosthetic = LegSocket->DetachProsthetic();
		}

		if (Prosthetic != nullptr)
		{
			NearbyInteractables.Add(Prosthetic);
		}
	}

	return Prosthetic;
}

AProsthetic* AErosCharacter::GetClosestProstheticOfType(EProstheticType Type)
{
	float ClosestDistance = FLT_MAX;
	AProsthetic* ClosestProsthetic = nullptr;

	for (IInteractableInterface* Interactable : NearbyInteractables)
	{
		if (AProsthetic* Prosthetic = Cast<AProsthetic>(Interactable))
		{
			float Distance = FVector::Dist(GetActorLocation(), Prosthetic->GetActorLocation());

			if ((ClosestProsthetic == nullptr || (Distance < ClosestDistance)) && Prosthetic->GetType() == Type)
			{
				ClosestProsthetic = Prosthetic;
				ClosestDistance = Distance;
			}
		}
	}
	return ClosestProsthetic;
}

IInteractableInterface* AErosCharacter::GetClosestInteractable()
{
	float ClosestDistance = FLT_MAX;
	IInteractableInterface* ClosestsInteractable = nullptr;

	for (IInteractableInterface* Interactable : NearbyInteractables)
	{
		float Distance = FVector::Dist(GetActorLocation(), Interactable->GetLocation());

		if ((ClosestsInteractable == nullptr || (Distance < ClosestDistance)))
		{
			ClosestsInteractable = Interactable;
			ClosestDistance = Distance;
		}
	}
	return ClosestsInteractable;
}

void AErosCharacter::SwapLegProsthetic()
{
	// Find the closest leg prosthetic
	AProsthetic* LegProsthetic = GetClosestProstheticOfType(EProstheticType::PT_Leg);

	// If there is no free socket, detach the current prosthetic
	if (!HasFreeSocketOfType(EProstheticType::PT_Leg))
	{
		DetachProstheticOfType(EProstheticType::PT_Leg)->GetMesh()->SetVisibility(true, true);
	}

	// Attach new prosthetic, if one is available
	if (LegProsthetic != nullptr)
	{
		AttachProsthetic(*LegProsthetic);
		LegProsthetic->GetMesh()->SetVisibility(!bIsCameraTooClose, true);
	}

	OnProstheticSwapped.Broadcast(this);
}

void AErosCharacter::SwapArmProsthetic()
{
	// Find the closest arm prosthetic
	AProsthetic* ArmProsthetic = GetClosestProstheticOfType(EProstheticType::PT_Arm);

	// If there is no free socket, detach the current prosthetic
	if (!HasFreeSocketOfType(EProstheticType::PT_Arm))
	{
		DetachProstheticOfType(EProstheticType::PT_Arm)->GetMesh()->SetVisibility(true, true);		
	}

	// Attach new prosthetic, if one is available
	if (ArmProsthetic != nullptr)
	{
		AttachProsthetic(*ArmProsthetic);
		ArmProsthetic->GetMesh()->SetVisibility(!bIsCameraTooClose, true);
	}
}

bool AErosCharacter::AttachProsthetic(AProsthetic& Prosthetic)
{
	if (ArmSocket->TryAttachProsthetic(Prosthetic) || LegSocket->TryAttachProsthetic(Prosthetic))
	{
		NearbyInteractables.Remove(&Prosthetic);
		return true;
	}

	return false;
}

void AErosCharacter::CheckLedgeGrab()
{
	// Note: these may need to use the Unscaled height and radius...
	float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();

	FVector LedgeImpactPoint = FVector::ZeroVector;
	FVector WallImpactPoint = FVector::ZeroVector;
	FVector WallImpactNormal = FVector::ZeroVector;

	// ledge tracing variables
	FHitResult TracerResult;
	FVector StartWallTrace = GetActorLocation() + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2);
	FVector EndWallTrace = StartWallTrace + GetActorForwardVector() * 50.0f;
	ECollisionChannel Channel = ECollisionChannel::ECC_GameTraceChannel2;
	FCollisionQueryParams Query;
	FCollisionResponseParams Response;
	FCollisionShape TracerShape = GetCapsuleComponent()->GetCollisionShape();
	TracerShape.Capsule.HalfHeight = CapsuleHalfHeight / 2;

#if false
	DrawDebugCapsule(GetWorld(), StartWallTrace, TracerShape.Capsule.HalfHeight, TracerShape.Capsule.Radius, FQuat(0.0f, 0.0f, 0.0f, 1.0f), FColor(255, 0, 0), false, -1, 0, 1.0f);
	DrawDebugCapsule(GetWorld(), (StartWallTrace+EndWallTrace)/2, TracerShape.Capsule.HalfHeight, TracerShape.Capsule.Radius, FQuat(0.0f, 0.0f, 0.0f, 1.0f), FColor(255, 0, 0), false, -1, 0, 1.0f);
	DrawDebugCapsule(GetWorld(), EndWallTrace, TracerShape.Capsule.HalfHeight, TracerShape.Capsule.Radius, FQuat(0.0f, 0.0f, 0.0f, 1.0f), FColor(255, 0, 0), false, -1, 0, 1.0f);
#endif

	// wall tracing
	if (GetWorld()->SweepSingleByChannel(TracerResult, StartWallTrace, EndWallTrace, FQuat(0,0,0,1), Channel, TracerShape, Query, Response) && 
		(GetCharacterMovement()->Velocity.Size() > 300 || CurrentState==ECharacterState::CS_WallRunVert) )
	{
#if false
		DrawDebugCapsule(GetWorld(), TracerResult.ImpactPoint, CapsuleHalfHeight, CapsuleRadius, FQuat(0.0f, 0.0f, 0.0f, 1.0f), FColor(0, 255, 0), false, -1, 0, 1.0f);
#endif
		
		WallImpactPoint = TracerResult.ImpactPoint;
		WallImpactNormal = TracerResult.ImpactNormal;

		FVector StartLedgeTrace = TracerResult.ImpactPoint + FVector(0.0f, 0.0f, 500.0f) + GetActorForwardVector();
		FVector EndLedgeTrace = StartLedgeTrace - FVector(0.0f, 0.0f, 550.0f);

#if false
		DrawDebugLine(GetWorld(), StartLedgeTrace, EndLedgeTrace, FColor(255, 0, 0), false, -1, 0, 12.333);
#endif

		// ledge tracing
		if (GetWorld()->LineTraceSingleByChannel(TracerResult, StartLedgeTrace, EndLedgeTrace, Channel, Query, Response))
		{
#if false
			DrawDebugCapsule(GetWorld(), TracerResult.ImpactPoint, CapsuleHalfHeight, CapsuleRadius, FQuat(0.0f, 0.0f, 0.0f, 1.0f), FColor(0, 255, 0), false, -1, 0, 1.0f);
#endif

			LedgeImpactPoint = TracerResult.ImpactPoint;

			// calculate distance from the ledge
			float DistFromLedge = (LedgeImpactPoint - GetActorLocation() + FVector(0, CapsuleHalfHeight, 0)).Size();

			// if ledge is just above character head, jump and grab it
			if (DistFromLedge >= CapsuleHalfHeight && DistFromLedge <= CapsuleHalfHeight * 2 && (CurrentState == ECharacterState::CS_OnGround || CurrentState == ECharacterState::CS_InAir || CurrentState == ECharacterState::CS_WallRunVert))
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Custom);
				CurrentState = ECharacterState::CS_Hanging;

				if (TracerResult.GetComponent()->ComponentHasTag("Wall"))
				{
					LedgeGrabTag = "Wall";
					UE_LOG(LogTemp, Log, TEXT("Tag set to Wall"));
				}
				else if (TracerResult.GetComponent()->ComponentHasTag("Ledge"))
				{
					LedgeGrabTag = "Ledge";
					UE_LOG(LogTemp, Log, TEXT("Tag set to Ledge"));
				}
				
				UE_LOG(LogTemp, Warning, TEXT("Ledge Trace: State changed to hanging"));

				float X = WallImpactPoint.X + (WallImpactNormal * CapsuleRadius).X;
				float Y = WallImpactPoint.Y + (WallImpactNormal * CapsuleRadius).Y;
				float Z = LedgeImpactPoint.Z - 65.0f;

				FVector TargetLocation = FVector(X, Y, Z);

				FVector RotationVector = WallImpactNormal * -1;
				RotationVector.Normalize();

				FLatentActionInfo LatentInfo;
				LatentInfo.CallbackTarget = this;

				UKismetSystemLibrary::MoveComponentTo(RootComponent, TargetLocation, RotationVector.Rotation(), false, false, 0.2f, false, EMoveComponentAction::Type::Move, LatentInfo);
				GetMovementComponent()->StopMovementImmediately();
			}
			// if ledge is not close enough, run up wall
			else if (DistFromLedge > CapsuleHalfHeight * 2.0f && DistFromLedge <= CapsuleHalfHeight * 4.0f && CurrentState == ECharacterState::CS_OnGround)
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Flying);
				CurrentState = ECharacterState::CS_WallRunVert;

				UE_LOG(LogTemp, Warning, TEXT("State changed to running up wall"));
				UE_LOG(LogTemp, Log, TEXT("Distance from ledge point is: %f"), DistFromLedge);
			}
		}
	}
}

void AErosCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IInteractableInterface* Interactable = Cast<IInteractableInterface>(OtherActor))
	{
		NearbyInteractables.Add(Interactable);
	}
}

void AErosCharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IInteractableInterface* Interactable = Cast<IInteractableInterface>(OtherActor))
	{
		NearbyInteractables.Remove(Interactable);
	}
}

void AErosCharacter::OnCollided(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor->GetClass()->IsChildOf(ADroneAICharacter::StaticClass()))
	{
		KillPlayer();
	}
}

void AErosCharacter::Interact()
{
	if (IInteractableInterface* Interactable = GetClosestInteractable())
	{
		Interactable->Interact(*this);
	}
}

FErosCharacterMovement AErosCharacter::GetModifiedCharacterMovement(FErosCharacterMovement DefaultMovement) const
{
	FErosCharacterMovement ModifiedMovement = DefaultMovement;

	if (ArmSocket->HasProsthetic())
	{
		ModifiedMovement = ArmSocket->GetProsthetic()->ApplyModifiers(ModifiedMovement);
	}

	if (LegSocket->HasProsthetic())
	{
		ModifiedMovement = LegSocket->GetProsthetic()->ApplyModifiers(ModifiedMovement);
	}

	return ModifiedMovement;
}

void AErosCharacter::UpdateCharacterMovement(FErosCharacterMovement Movement)
{
	GetCharacterMovement()->AirControl = Movement.AirControl;
	GetCharacterMovement()->JumpZVelocity = Movement.JumpZVelocity;
	GetCharacterMovement()->MaxWalkSpeed = Movement.MaxWalkSpeed;
	GetCharacterMovement()->RotationRate = Movement.RotationRate;
}

void AErosCharacter::CheckFallDamage()
{
	if (GetCharacterMovement()->IsFalling())
	{
		if (!bWasFalling)
		{
			bWasFalling = true;
			FallStartPosition = GetActorLocation();
		}
		else if (GetActorLocation().Z > FallStartPosition.Z)
		{
			FallStartPosition = GetActorLocation();
		}
	}
	else if (!GetCharacterMovement()->IsFalling() && bWasFalling)
	{
		bWasFalling = false;
		float FallDistance = GetActorLocation().Z - FallStartPosition.Z;

		UE_LOG(LogTemp, Warning, TEXT("Fall distance: %f"), FallDistance);

		if (FallDistance < MinFallDistance)
		{
			if (FallDistance < MaxFallDistance)
			{
				KillPlayer();
			}
			else
			{
				// TODO: Fall damage effect!
				// Gives a value between 0.0 - 1.0
				float FallDamagePercent = (FallDistance - MinFallDistance) / (MaxFallDistance - MinFallDistance);

				UE_LOG(LogTemp, Warning, TEXT("Fall damage: %f"), FallDamagePercent);
			}
		}
	}
}

void AErosCharacter::KillPlayer()
{
	UE_LOG(LogTemp, Warning, TEXT("THE PLAYER HAS DIED FOR BEING A FUCKIN DINGUS!"));
}

void AErosCharacter::UpdateCharacterState()
{
	// Make character crawl if they only have one leg
	if (HasFreeSocketOfType(EProstheticType::PT_Leg))
	{
		CurrentState = ECharacterState::CS_Crawling;
	}
	else if (CurrentState == ECharacterState::CS_Crawling)
	{
		CurrentState = ECharacterState::CS_OnGround;
	}

	// keep movement mode inline with CurrentState
	if (GetCharacterMovement()->MovementMode != MovementModeLastTick)
	{
		if (GetCharacterMovement()->MovementMode == MOVE_Walking)
		{
			CurrentState = ECharacterState::CS_OnGround;
		}
		else if (GetCharacterMovement()->MovementMode == MOVE_Falling)
		{
			switch (CurrentState)
			{
			case ECharacterState::CS_Dropping:
				break;
			default:
				CurrentState = ECharacterState::CS_InAir;
				break;
			}
		}
	}

	// apply movement mode changes based on CurrentState
	if (CurrentState != StateLastTick)
	{
		switch (CurrentState)
		{
		case ECharacterState::CS_OnGround:
			DefaultCharacterMovement.MaxWalkSpeed = 600.0f;
			GetCapsuleComponent()->SetCapsuleHalfHeight(93);
			GetMesh()->SetRelativeLocation(FVector(0, 0, -93));
			UE_LOG(LogTemp, Warning, TEXT("MaxWalkSpeed set to 600"));
			LedgeGrabTag = "";
			break;
		case ECharacterState::CS_Crouching:
			DefaultCharacterMovement.MaxWalkSpeed = 400.0f;
			GetCapsuleComponent()->SetCapsuleHalfHeight(63);
			GetMesh()->SetRelativeLocation(FVector(0, 0, -63));
			UE_LOG(LogTemp, Warning, TEXT("MaxWalkSpeed set to 400"));
			break;
		case ECharacterState::CS_Crawling:
			DefaultCharacterMovement.MaxWalkSpeed = 200.0f;
			UE_LOG(LogTemp, Warning, TEXT("MaxWalkSpeed set to 200"));
			break;
		default:
			break;
		}
	}
}

void AErosCharacter::CheckCameraDistance()
{
	bIsCameraTooClose = (CameraBoom->GetComponentLocation() - FollowCamera->GetComponentLocation()).Size() < 125.0f;

	GetMesh()->SetVisibility(!bIsCameraTooClose, true);
}