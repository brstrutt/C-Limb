#include "Eros.h"

#include "../AI/DroneAICharacter.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "DrawDebugHelpers.h"
#include "ErosCharacter.h"
#include "ErosController.h"
#include "Misc/ErosGameMode.h"
#include "Prosthetics/ProstheticSocket.h"
#include "Prosthetics/Prosthetic.h"
#include "Prosthetics/UnattachedProsthetic.h"
#include "Prosthetics/Prosthetic_GrappleArm.h"
#include "Sound/SoundCue.h"
#include "InteractableObjects/Switch/Switch.h"
#include "InteractableObjects/ProstheticCabinet.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>

#define STANDING_CAPSULE_HALF_HEIGHT 93
#define CROUCHING_CAPSULE_HALF_HEIGHT 63

AErosCharacter::AErosCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

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

	ArmSocket = CreateDefaultSubobject<UProstheticSocket>(TEXT("ArmSocket"));
	ArmSocket->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	LegSocket = CreateDefaultSubobject<UProstheticSocket>(TEXT("LegSocket"));
	LegSocket->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	CharacterVoice = CreateDefaultSubobject<UAudioComponent>(TEXT("CharacterVoice"));
	CharacterVoice->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	// Prosthetics pickup zone.
	InteractZone = CreateDefaultSubobject<USphereComponent>(TEXT("InteractZone"));
	InteractZone->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	// Delegate for moving into prosthetic pickup range.
	FScriptDelegate ProstheticInRangeDel;
	ProstheticInRangeDel.BindUFunction(this, FName("OnOverlapBegin"));
	InteractZone->OnComponentBeginOverlap.Add(ProstheticInRangeDel);

	// Delegate for moving out of prosthetic pickup range.
	FScriptDelegate ProstheticOutOfRangeDel;
	ProstheticOutOfRangeDel.BindUFunction(this, FName("OnOverlapEnd"));
	InteractZone->OnComponentEndOverlap.Add(ProstheticOutOfRangeDel);

	// Delegate for overlapping with the character collider
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AErosCharacter::OnCapsuleOverlapBegin);

	// PawnNoiseEmitter is required to generate Noise events that can be detected by the AI
	LocalNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("LocalNoiseEmitterComp"));

	// Create audio component
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	// Default footstep volume.
	FootstepVolume = 0.3f;	

	// Delegate for collisions with the core body of the character
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AErosCharacter::OnCollided);

	PreviousNearestLeg = nullptr;
	PreviousNearestArm = nullptr;
	PreviousNearestButton = nullptr;
}

void AErosCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Spawn and attach default limbs.
	AttachProsthetic(*GetWorld()->SpawnActor<AProsthetic>(DefaultArm.GetDefaultObject()->GetClass()));
	AttachProsthetic(*GetWorld()->SpawnActor<AProsthetic>(DefaultLeg.GetDefaultObject()->GetClass()));

	UE_LOG(LogTemp, Warning, TEXT("BeginPlay"));
}

void AErosCharacter::Say(USoundCue* Voiceline)
{
	CharacterVoice->SetSound(Voiceline);
	CharacterVoice->Play();
}

void AErosCharacter::MakeProstheticFootstepNoise()
{
	if (LegSocket->HasProsthetic())
	{
		// Play the audio clip
		LegSocket->GetProsthetic()->PlayFootstepSound();
		
		LocalNoiseEmitter->MakeNoise(this, 1.0f, GetActorLocation());
	}
}

void AErosCharacter::MakeRegularFootstepNoise()
{
	if (FootstepSound == nullptr) { return; }

	AudioComponent->SetVolumeMultiplier(FootstepVolume);
	AudioComponent->SetSound(FootstepSound);
	AudioComponent->Play();

	LocalNoiseEmitter->MakeNoise(this, 1.0f, GetActorLocation());
}

float AErosCharacter::GetMaxVolume() const
{
	if (LegSocket->HasProsthetic())
	{
		return LegSocket->GetProsthetic()->GetSoundVolume();
	}

	return FootstepVolume;
}

void AErosCharacter::Tick(float DeltaSeconds)
{
	TickState(DeltaSeconds);

	CheckLedgeGrab();
	CheckFallDamage();
	UpdateFallDamage(DeltaSeconds);
	CheckCameraDistance();
	
	UpdateInteractableHighlights();

	UpdateCharacterMovement(GetModifiedCharacterMovement(DefaultCharacterMovement));

	if (CurrentState == ECharacterState::CS_Aiming)
	{
		FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(FRotator(0,GetControlRotation().Yaw,0),FRotator(0,GetActorRotation().Yaw,0));

		if (FMath::Abs(DeltaRotator.Yaw)>=80)
		{
			AddActorWorldRotation(FMath::RInterpTo(FRotator(0,0,0), DeltaRotator, DeltaSeconds, 5));
		}
	}
}

void AErosCharacter::Jump()
{
	switch (CurrentState)
	{
	case ECharacterState::CS_Hanging:
		if (HasProstheticOfType(EProstheticType::PT_Arm))
		{
			ChangeState(ECharacterState::CS_Climbing);
		}
		break;
	case ECharacterState::CS_Crawling:
	case ECharacterState::CS_LegOff:
	case ECharacterState::CS_LegOn:
	case ECharacterState::CS_ArmOff:
	case ECharacterState::CS_ArmOn:
	case ECharacterState::CS_StandUp:
	case ECharacterState::CS_StartCrawl:
		// cant jump when laying on floor
		break;
	default:
		bPressedJump = true;
		JumpKeyHoldTime = 0.0f;
		break;
	}
}

void AErosCharacter::Crouch(bool bClientSimulation)
{
	switch (CurrentState)
	{
	case ECharacterState::CS_Hanging:
		ChangeState(ECharacterState::CS_Dropping);
		break;

	case ECharacterState::CS_OnGround:
		ChangeState(ECharacterState::CS_Crouching);
		break;

	case ECharacterState::CS_Crouching:
		FHitResult outhit;
		FVector CapsuleCentre = GetActorLocation() - FVector(0, 0, CROUCHING_CAPSULE_HALF_HEIGHT) + FVector(0, 0, STANDING_CAPSULE_HALF_HEIGHT);
		FCollisionShape shape = GetCapsuleComponent()->GetCollisionShape();
		shape.Capsule.HalfHeight = STANDING_CAPSULE_HALF_HEIGHT;

		if (!GetWorld()->SweepSingleByObjectType(outhit, CapsuleCentre, CapsuleCentre, FQuat(0,0,0,1), FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldStatic), shape))// || GetWorld()->SweepSingleByChannel(outhit, CapsuleCentre, CapsuleCentre, FQuat(0, 0, 0, 1), ECollisionChannel::ECC_GameTraceChannel5, shape))
		{
			ChangeState(ECharacterState::CS_OnGround);
		}
		break;
	}
}

void AErosCharacter::MoveForward(float Value)
{
	if (!Value) { return; }

	switch (CurrentState)
	{
	case ECharacterState::CS_Hanging:
		if (HasProstheticOfType(EProstheticType::PT_Arm) && Value > 0.5)
		{
			ChangeState(ECharacterState::CS_Climbing);
		}
		break;
	case ECharacterState::CS_LegOff:
	case ECharacterState::CS_LegOn:
	case ECharacterState::CS_ArmOff:
	case ECharacterState::CS_ArmOn:
	case ECharacterState::CS_StandUp:
	case ECharacterState::CS_StartCrawl:
	case ECharacterState::CS_CraneHang:
		break;

	default:
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
		break;

	}
}

void AErosCharacter::MoveRight(float Value)
{
	if (!Value) { return; }

	switch (CurrentState)
	{
	case ECharacterState::CS_Hanging:
	case ECharacterState::CS_LegOff:
	case ECharacterState::CS_LegOn:
	case ECharacterState::CS_ArmOff:
	case ECharacterState::CS_ArmOn:
	case ECharacterState::CS_StandUp:
	case ECharacterState::CS_StartCrawl:
	case ECharacterState::CS_CraneHang:
		break;

	default:
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
		break;
	}
}

void AErosCharacter::PrimaryActionBegin()
{
	// Only arm prosthetics have a primary action.
	if (ArmSocket->HasProsthetic() && CanUseProsthetic())
	{
		ArmSocket->GetProsthetic()->PrimaryActionBegin();
	}
}

void AErosCharacter::SecondaryActionBegin()
{
	// Only arm prosthetics have a secondary action.
	if (ArmSocket->HasProsthetic() && CanUseProsthetic())
	{
		ArmSocket->GetProsthetic()->SecondaryActionBegin();
	}
}

void AErosCharacter::PrimaryActionEnd()
{
	if (ArmSocket->HasProsthetic() & CanUseProsthetic())
	{
		ArmSocket->GetProsthetic()->PrimaryActionEnd();
	}
}

void AErosCharacter::SecondaryActionEnd()
{
	if (ArmSocket->HasProsthetic() && CanUseProsthetic())
	{
		ArmSocket->GetProsthetic()->SecondaryActionEnd();
	}
}

bool AErosCharacter::HasProstheticAttached(UClass const* Class) const
{
	return ((ArmSocket->HasProsthetic() && ArmSocket->GetProsthetic()->IsA(Class)) || (LegSocket->HasProsthetic() && LegSocket->GetProsthetic()->IsA(Class)));
}

bool AErosCharacter::HasProstheticOfType(EProstheticType Type) const
{
	return ((ArmSocket->GetProstheticType() == Type && ArmSocket->HasProsthetic()) || (LegSocket->GetProstheticType() == Type && LegSocket->HasProsthetic()));
}

AProsthetic* AErosCharacter::DetachProstheticOfType(EProstheticType Type)
{
	AProsthetic* Prosthetic = nullptr;

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

	return Prosthetic;
}

AUnattachedProsthetic* AErosCharacter::GetClosestProstheticOfType(EProstheticType Type)
{
	float ClosestDistance = FLT_MAX;
	AUnattachedProsthetic* ClosestProsthetic = nullptr;

	for (AInteractableActor* Interactable : NearbyInteractables)
	{
		if (AUnattachedProsthetic* Prosthetic = Cast<AUnattachedProsthetic>(Interactable))
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

AInteractableActor* AErosCharacter::GetClosestInteractable() const
{
	float ClosestDistance = FLT_MAX;
	AInteractableActor* ClosestsInteractable = nullptr;

	for (AInteractableActor* Interactable : NearbyInteractables)
	{
		float Distance = FVector::Dist(GetActorLocation(), Interactable->GetActorLocation());

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
	if (CurrentState != ECharacterState::CS_OnGround && CurrentState != ECharacterState::CS_Crawling) { return; }

	// see if a new prosthetic is near
	NewLegProstheticToAttach = GetClosestProstheticOfType(EProstheticType::PT_Leg);

	if (NewLegProstheticToAttach != nullptr)
	{
		NewLegProstheticToAttach->SetInteractable(false);

		if (NewLegProstheticToAttach == DroppedLegProsthetic)
		{
			DroppedLegProsthetic = nullptr;
		}

		if (HasProstheticOfType(EProstheticType::PT_Leg))
		{
			ChangeState(ECharacterState::CS_LegOff);
		}
		else
		{
			AttachLegProsthetic();
		}
	}
}
void AErosCharacter::DetachLegProsthetic()
{
	if (HasProstheticOfType(EProstheticType::PT_Leg))
	{
		AProsthetic* LegProsthetic = DetachProstheticOfType(EProstheticType::PT_Leg);

		LegProstheticToRemove = GetWorld()->SpawnActor<AUnattachedProsthetic>(LegProsthetic->GetUnattachedProsthetic().GetDefaultObject()->GetClass());

		LegProstheticToRemove->CanSimulatePhysics(false);
		LegProstheticToRemove->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("HeldLegSocket"));
		LegProstheticToRemove->SetUIVisible(false);
		LegProstheticToRemove->SetInteractable(false);

		LegProsthetic->Destroy();
	}
	else
	{
		AttachLegProsthetic();
	}
}
void AErosCharacter::LegProstheticDetached()
{
	// drop old prosthetic
	if (LegProstheticToRemove != nullptr)
	{
		DroppedLegProsthetic = Cast<AUnattachedProsthetic>(LegProstheticToRemove);
		DroppedLegProsthetic->CanSimulatePhysics(true);
		DroppedLegProsthetic->SetActorLocation(GetActorLocation() + FVector(0, 0, 20));
		DroppedLegProsthetic->OnDetached();

		LegProstheticToRemove->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		LegProstheticToRemove = nullptr;
	}

	// attach a new prosthetic or switch to crawling?
	if (NewLegProstheticToAttach != nullptr)
	{
		// attach new prosthetic
		AttachLegProsthetic();
	}
	else
	{
		ChangeState(ECharacterState::CS_StartCrawl);
	}
}
void AErosCharacter::AttachLegProsthetic()
{
	if (NewLegProstheticToAttach != nullptr)
	{
		// attach prosthetic to hand bone
		NewLegProstheticToAttach->CanSimulatePhysics(false);
		NewLegProstheticToAttach->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("HeldLegSocket"));

		ChangeState(ECharacterState::CS_LegOn);
	}
}
void AErosCharacter::LegProstheticAttached()
{
	AActor* HeldLeg = GetHeldProstheticOfType(EProstheticType::PT_Leg);
	if (HeldLeg != nullptr)
	{		
		// Re-activate the dropped prosthetic
		if (DroppedLegProsthetic != nullptr)
		{
			DroppedLegProsthetic->SetInteractable(true);
		}

		AUnattachedProsthetic* ProstheticBeingHeld = Cast<AUnattachedProsthetic>(HeldLeg);
		ProstheticBeingHeld->OnAttached();

		// Is required in OnAttached, must be set null after.
		DroppedLegProsthetic = nullptr;

		UClass* ProstheticToAttach = ProstheticBeingHeld->GetProsthetic().GetDefaultObject()->GetClass();

		// spawn and attach new prosthetic
		AProsthetic * LegProsthetic = GetWorld()->SpawnActor<AProsthetic>(ProstheticToAttach);
		AttachProsthetic(*LegProsthetic);
		//LegProsthetic->GetMesh()->SetVisibility(!bIsCameraTooClose);
		LegProsthetic->SetMeshVisibility(!bIsCameraTooClose);

		// delete the unattached version
		NearbyInteractables.Remove(ProstheticBeingHeld);
		PreviousNearestLeg = nullptr;
		ProstheticBeingHeld->Destroy();

		NewLegProstheticToAttach = nullptr;
		OnProstheticSwapped.Broadcast(this, LegProsthetic);
		ChangeState(ECharacterState::CS_StandUp);
	}
}

void AErosCharacter::SwapArmProsthetic()
{
	if (CurrentState != ECharacterState::CS_OnGround && CurrentState != ECharacterState::CS_Crawling) { return; }

	// see if a new prosthetic is near
	NewArmProstheticToAttach = GetClosestProstheticOfType(EProstheticType::PT_Arm);

	if (NewArmProstheticToAttach != nullptr)
	{
		NewArmProstheticToAttach->SetInteractable(false);

		if (NewArmProstheticToAttach == DroppedArmProsthetic)
		{
			NewArmProstheticToAttach = nullptr;
		}

		if (HasProstheticOfType(EProstheticType::PT_Arm))
		{
			ChangeState(ECharacterState::CS_ArmOff);
		}
		else
		{
			AttachArmProsthetic();
		}
	}
}
void AErosCharacter::DetachArmProsthetic()
{
	if (HasProstheticOfType(EProstheticType::PT_Arm))
	{
		AProsthetic* ArmProsthetic = DetachProstheticOfType(EProstheticType::PT_Arm);

		ArmProstheticToRemove = GetWorld()->SpawnActor<AUnattachedProsthetic>(ArmProsthetic->GetUnattachedProsthetic().GetDefaultObject()->GetClass());

		ArmProstheticToRemove->CanSimulatePhysics(false);
		ArmProstheticToRemove->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("HeldArmSocket"));
		ArmProstheticToRemove->SetUIVisible(false);
		ArmProstheticToRemove->SetInteractable(false);

		ArmProsthetic->Destroy();
	}
	else
	{
		AttachArmProsthetic();
	}
}
void AErosCharacter::ArmProstheticDetached()
{
	// drop old prosthetic
	if (ArmProstheticToRemove != nullptr)
	{
		DroppedArmProsthetic = Cast<AUnattachedProsthetic>(ArmProstheticToRemove);
		DroppedArmProsthetic->CanSimulatePhysics(true);
		DroppedArmProsthetic->SetActorLocation(GetActorLocation()+FVector(0,0,20));
		DroppedArmProsthetic->OnDetached();

		ArmProstheticToRemove->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		ArmProstheticToRemove = nullptr;
	}

	// attach a new prosthetic?
	if (NewArmProstheticToAttach != nullptr)
	{
		// attach new prosthetic
		AttachArmProsthetic();
	}
	else
	{
		if (HasProstheticOfType(EProstheticType::PT_Leg))
		{
			ChangeState(ECharacterState::CS_StandUp);
		}
		else
		{
			ChangeState(ECharacterState::CS_StartCrawl);
		}
	}
}
void AErosCharacter::AttachArmProsthetic()
{
	if (NewArmProstheticToAttach != nullptr)
	{
		// attach prosthetic to hand bone
		NewArmProstheticToAttach->CanSimulatePhysics(false);
		NewArmProstheticToAttach->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("HeldArmSocket"));

		ChangeState(ECharacterState::CS_ArmOn);
	}
}
void AErosCharacter::ArmProstheticAttached()
{
	AActor* HeldArm = GetHeldProstheticOfType(EProstheticType::PT_Arm);
	if (HeldArm != nullptr)
	{
		// Re-activate the dropped prosthetic
		if (DroppedArmProsthetic != nullptr)
		{
			DroppedArmProsthetic->SetInteractable(true);
		}

		AUnattachedProsthetic* ProstheticBeingHeld = Cast<AUnattachedProsthetic>(HeldArm);
		ProstheticBeingHeld->OnAttached();

		// Is required in OnAttached, must be set null after.
		DroppedArmProsthetic = nullptr;

		UClass* ProstheticToAttach = ProstheticBeingHeld->GetProsthetic().GetDefaultObject()->GetClass();
		
		// spawn and attach new prosthetic
		AProsthetic * ArmProsthetic = GetWorld()->SpawnActor<AProsthetic>(ProstheticToAttach);
		AttachProsthetic(*ArmProsthetic);
		//ArmProsthetic->GetMesh()->SetVisibility(!bIsCameraTooClose);
		ArmProsthetic->SetMeshVisibility(!bIsCameraTooClose);

		// delete the unattached version
		NearbyInteractables.Remove(ProstheticBeingHeld);
		PreviousNearestArm = nullptr;
		ProstheticBeingHeld->Destroy();

		NewArmProstheticToAttach = nullptr;
		OnProstheticSwapped.Broadcast(this, ArmProsthetic);
	}

	if (HasProstheticOfType(EProstheticType::PT_Leg))
	{
		ChangeState(ECharacterState::CS_StandUp);
	}
	else
	{
		ChangeState(ECharacterState::CS_StartCrawl);
	}
}

AActor* AErosCharacter::GetHeldProstheticOfType(EProstheticType ProstheticType)
{
	AActor* HeldProsthetic = nullptr;
	for (int i = 0; i<GetMesh()->GetAttachChildren().Num(); i++)
	{
		USceneComponent * ComponentToCheck = GetMesh()->GetAttachChildren()[i];
		AUnattachedProsthetic * CheckIfProsthetic = Cast<AUnattachedProsthetic>(ComponentToCheck->GetOwner());
		if (CheckIfProsthetic != nullptr && CheckIfProsthetic->GetType() == ProstheticType)
		{
			HeldProsthetic = ComponentToCheck->GetOwner();
			i = GetMesh()->GetAttachChildren().Num();
		}
	}
	return HeldProsthetic;
}

bool AErosCharacter::CanUseProsthetic() const
{
	return CurrentState == ECharacterState::CS_OnGround || CurrentState == ECharacterState::CS_Aiming;
}

bool AErosCharacter::AttachProsthetic(AProsthetic& Prosthetic)
{
	if (ArmSocket->TryAttachProsthetic(Prosthetic) || LegSocket->TryAttachProsthetic(Prosthetic))
	{
		NearbyInteractables.Remove(&Prosthetic);

		// Apply prosthetic modifiers.
		UpdateCharacterMovement(GetModifiedCharacterMovement(DefaultCharacterMovement));

		return true;
	}

	return false;
}

void AErosCharacter::CheckLedgeGrab()
{
	// Note: these may need to use the Unscaled height and radius...
	float const CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float const CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();

	// ledge tracing variables
	FCollisionShape TracerShape = GetCapsuleComponent()->GetCollisionShape();
	TracerShape.Capsule.HalfHeight = CapsuleHalfHeight / 2;

	FVector const StartWallTrace = GetActorLocation() + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2);
	FVector const EndWallTrace = StartWallTrace + GetActorForwardVector() * 50.0f;
	ECollisionChannel const Channel = ECollisionChannel::ECC_GameTraceChannel2;

	FHitResult TracerResult;
	FCollisionQueryParams Query;
	FCollisionResponseParams Response;

	// wall tracing
	if (GetWorld()->SweepSingleByChannel(TracerResult, StartWallTrace, EndWallTrace, FQuat(0.0f, 0.0f, 0.0f, 1.0f), Channel, TracerShape, Query, Response)) // && 
		/* (GetCharacterMovement()->Velocity.Size() > 300.0f || CurrentState == ECharacterState::CS_WallRunVert || CurrentState == ECharacterState::CS_InAir) ) */
	{		
		FVector const WallImpactPoint = TracerResult.ImpactPoint;
		FVector const WallImpactNormal = TracerResult.ImpactNormal;

		FVector const StartLedgeTrace = TracerResult.ImpactPoint + FVector(0.0f, 0.0f, 500.0f) + GetActorForwardVector();
		FVector const EndLedgeTrace = StartLedgeTrace - FVector(0.0f, 0.0f, 550.0f);

		// ledge tracing
		if (GetWorld()->LineTraceSingleByChannel(TracerResult, StartLedgeTrace, EndLedgeTrace, Channel, Query, Response))
		{
			FVector const LedgeImpactPoint = TracerResult.ImpactPoint;

			// calculate distance from the ledge
			float const DistFromLedge = (LedgeImpactPoint - GetActorLocation() + FVector(0, CapsuleHalfHeight, 0)).Size();

			// if ledge is just above character head, jump and grab it
			if (DistFromLedge >= CapsuleHalfHeight && DistFromLedge <= CapsuleHalfHeight * 2.0f && (CurrentState == ECharacterState::CS_OnGround || CurrentState == ECharacterState::CS_InAir))
			{
				ChangeState(ECharacterState::CS_Hanging);

				if (TracerResult.GetComponent()->ComponentHasTag("Ledge"))
				{
					LedgeGrabTag = "Ledge";
				}
				else
				{
					LedgeGrabTag = "Wall";
				}
				
				float X = WallImpactPoint.X + (WallImpactNormal * CapsuleRadius).X;
				float Y = WallImpactPoint.Y + (WallImpactNormal * CapsuleRadius).Y;
				float Z = LedgeImpactPoint.Z - 65.0f;

				FVector TargetLocation = FVector(X, Y, Z);

				FVector RotationVector = WallImpactNormal * -1;
				RotationVector.Normalize();
				FRotator TargetRotation = RotationVector.Rotation();
				TargetRotation.Pitch = 0;
				TargetRotation.Roll = 0;

				FLatentActionInfo LatentInfo;
				LatentInfo.CallbackTarget = this;

				UKismetSystemLibrary::MoveComponentTo(RootComponent, TargetLocation, TargetRotation, false, false, 0.2f, false, EMoveComponentAction::Type::Move, LatentInfo);
				GetMovementComponent()->StopMovementImmediately();
			}
		}
	}
}

void AErosCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) { return; }
	
	if (AInteractableActor* Interactable = Cast<AInteractableActor>(OtherActor))
	{
		if (!Cast<UMeshComponent>(OtherComp) && !OtherComp->ComponentHasTag("ErosCharIgnore"))
		{
			NearbyInteractables.Add(Interactable);
		}
	}
}

void AErosCharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AInteractableActor* Interactable = Cast<AInteractableActor>(OtherActor))
	{
		if (!Cast<UMeshComponent>(OtherComp) && !OtherComp->ComponentHasTag("ErosCharIgnore"))
		{
			NearbyInteractables.Remove(Interactable);
		}
	}
}

void AErosCharacter::OnCapsuleOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) { return; }

	if (Cast<ADroneAICharacter>(OtherActor))
	{
		KillPlayer();
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
	if (NearestButton != nullptr)
	{
		NearestButton->Interact(*this);
	}
}

FErosCharacterMovement AErosCharacter::GetModifiedCharacterMovement(FErosCharacterMovement DefaultMovement) const
{
	FErosCharacterMovement ModifiedMovement = DefaultMovement;

	// Apply modifiers from the prosthetics.
	if (ArmSocket->HasProsthetic())
	{
		ModifiedMovement = ArmSocket->GetProsthetic()->ApplyModifiers(ModifiedMovement);
	}

	if (LegSocket->HasProsthetic())
	{
		ModifiedMovement = LegSocket->GetProsthetic()->ApplyModifiers(ModifiedMovement);
	}

	// Apply modifiers from fall damage - reduce the speed by the fall damage percent
	ModifiedMovement.CrawlSpeed -= (ModifiedMovement.CrawlSpeed * FallDamagePercent);
	ModifiedMovement.CrouchSpeed -= (ModifiedMovement.CrouchSpeed * FallDamagePercent);
	ModifiedMovement.WalkSpeed -= (ModifiedMovement.WalkSpeed * FallDamagePercent);

	return ModifiedMovement;
}

void AErosCharacter::UpdateCharacterMovement(FErosCharacterMovement Movement)
{
	GetCharacterMovement()->AirControl = Movement.AirControl;
	GetCharacterMovement()->JumpZVelocity = Movement.JumpZVelocity;
	GetCharacterMovement()->RotationRate = Movement.RotationRate;

	switch (CurrentState)
	{
	case ECharacterState::CS_Crouching:
		GetCharacterMovement()->MaxWalkSpeed = Movement.CrouchSpeed;
		break;
	case ECharacterState::CS_Crawling:
		GetCharacterMovement()->MaxWalkSpeed = Movement.CrawlSpeed;
		break;
	default:
		GetCharacterMovement()->MaxWalkSpeed = Movement.WalkSpeed;
		break;
	}
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
		float const FallDistance = GetActorLocation().Z - FallStartPosition.Z;
		bWasFalling = false;

		UE_LOG(LogTemp, Warning, TEXT("Fall distance: %f"), FallDistance);

		if (FallDistance < MinFallDistance)
		{
			if (FallDistance < MaxFallDistance)
			{
				KillPlayer();
			}
			else
			{
				// Gives a value between 0.0 - 1.0
				FallDamagePercent = (FallDistance - MinFallDistance) / (MaxFallDistance - MinFallDistance);

				OnFallDamageChanged.Broadcast(FallDamagePercent);

				UE_LOG(LogTemp, Warning, TEXT("Fall damage: %f"), FallDamagePercent);
			}
		}
	}
}

void AErosCharacter::UpdateFallDamage(float DeltaSeconds)
{
	if (FallDamagePercent == 0.0f) { return; }

	FallDamagePercent -= (FallDamageRecovery * DeltaSeconds);

	if (FallDamagePercent < 0.0f)
	{
		FallDamagePercent = 0.0f;
	}

	OnFallDamageChanged.Broadcast(FallDamagePercent);
}

void AErosCharacter::KillPlayer()
{
	UE_LOG(LogTemp, Warning, TEXT("KillPlayer"));
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Destroy();
}

void AErosCharacter::Destroyed()
{
	AProsthetic* CurrentArm = nullptr;
	AProsthetic* CurrentLeg = nullptr;

	CurrentArm = ArmSocket->DetachProsthetic();
	CurrentLeg = LegSocket->DetachProsthetic();

	UE_LOG(LogTemp, Warning, TEXT("BEGIN DESTROOOOOY!"));
	if (CurrentArm)
	{
		UE_LOG(LogTemp, Warning, TEXT("Limb 1!"));
		CurrentArm->Destroy();
	}
	if (CurrentLeg)
	{
		UE_LOG(LogTemp, Warning, TEXT("Limb 2!"));
		CurrentLeg->Destroy();
	}

	Super::Destroyed();
}

void AErosCharacter::FuckIFellAndBrokeEverything()
{
	UE_LOG(LogTemp, Warning, TEXT("Prosthetic break!"));

	if (HasProstheticOfType(EProstheticType::PT_Arm))
	{
		ArmSocket->DetachProsthetic()->Destroy();
	}
	if (HasProstheticOfType(EProstheticType::PT_Leg))
	{
		LegSocket->DetachProsthetic()->Destroy();
	}
	bWasFalling = false;

	// This is only called at the bottom of the factory right now, and because the character
	// is likely hanging before hit the break volume they need to change to the drop state first
	// before then changing to the crawl state...
	ChangeState(CurrentState == ECharacterState::CS_Hanging ? ECharacterState::CS_Dropping : ECharacterState::CS_Crawling);
}

void AErosCharacter::ResetFallDistance()
{
	FallStartPosition = GetActorLocation();
}

void AErosCharacter::ResetForNewGame()
{	
	FVector const StartLocation = FVector(-391.0f, 25489.0f, 5989.574219f);
	FRotator const StartRotation = FRotator(0,-149,0);
		
	SetActorLocation(StartLocation);
	SetActorRotation(StartRotation);
	
	Cast<AErosGameMode>(GetWorld()->GetAuthGameMode())->UpdateRespawnInformation(this, this);

	// remove old limbs
	if (HasProstheticOfType(EProstheticType::PT_Arm))
	{
		DetachProstheticOfType(EProstheticType::PT_Arm)->Destroy();
	}
	if (HasProstheticOfType(EProstheticType::PT_Leg))
	{
		DetachProstheticOfType(EProstheticType::PT_Leg)->Destroy();
	}

	// Spawn and attach default limbs.
	AttachProsthetic(*GetWorld()->SpawnActor<AProsthetic>(DefaultArm.GetDefaultObject()->GetClass()));
	AttachProsthetic(*GetWorld()->SpawnActor<AProsthetic>(DefaultLeg.GetDefaultObject()->GetClass()));
}

void AErosCharacter::CheckCameraDistance()
{
	bIsCameraTooClose = (CameraBoom->GetComponentLocation() - FollowCamera->GetComponentLocation()).Size() < 125.0f;

	// Don't want to propogate the visibility because it turns some things off which we want on.

	GetMesh()->SetVisibility(!bIsCameraTooClose);

	if (HasProstheticOfType(EProstheticType::PT_Arm))
	{
		//GetArmSocket()->GetProsthetic()->GetMesh()->SetVisibility(!bIsCameraTooClose);
		GetArmSocket()->GetProsthetic()->SetMeshVisibility(!bIsCameraTooClose);
	}
	if (HasProstheticOfType(EProstheticType::PT_Leg))
	{
		//GetLegSocket()->GetProsthetic()->GetMesh()->SetVisibility(!bIsCameraTooClose);
		GetLegSocket()->GetProsthetic()->SetMeshVisibility(!bIsCameraTooClose);
	}
}

void AErosCharacter::ChangeState(ECharacterState NewState)
{
	if (NewState == CurrentState) { return; }

	const UEnum* NewStateEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ECharacterState"));
	UE_LOG(LogTemp, Log, TEXT("Changed State to %s"), *(NewStateEnum ? NewStateEnum->GetEnumName(static_cast<int>(NewState)) : TEXT("<Invalid Enum>")));

	ExitState(CurrentState);
	EnterState(NewState);
}

void AErosCharacter::EnterState(ECharacterState State)
{
	switch (State)
	{

	case ECharacterState::CS_OnGround:
	{

		float PrevCapsuleHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
		float CapsuleHeightDifference = STANDING_CAPSULE_HALF_HEIGHT - PrevCapsuleHeight;
		GetCapsuleComponent()->SetCapsuleHalfHeight(STANDING_CAPSULE_HALF_HEIGHT);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -STANDING_CAPSULE_HALF_HEIGHT));
		SetActorLocation(GetActorLocation() + FVector(0, 0, CapsuleHeightDifference));
		CameraBoom->SocketOffset = FVector(0, 0, 100);
		CameraBoom->TargetOffset = FVector(0, 0, 50);
		CameraBoom->TargetArmLength = 500;
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		LedgeGrabTag = "";
	}
		break;

	case ECharacterState::CS_Hanging:
	case ECharacterState::CS_CraneHang:

		GetCharacterMovement()->SetMovementMode(MOVE_Custom);

		break;

	case ECharacterState::CS_Dropping:

		GetCharacterMovement()->SetMovementMode(MOVE_Falling);

		break;

	case ECharacterState::CS_Climbing:

		GetCharacterMovement()->SetMovementMode(MOVE_Flying);

		break;

	case ECharacterState::CS_WallRunVert:

		GetCharacterMovement()->SetMovementMode(MOVE_Flying);

		break;

	case ECharacterState::CS_Crouching:
	{

		ACharacter::Crouch(false);

		float PrevCapsuleHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
		float CapsuleHeightDifference = CROUCHING_CAPSULE_HALF_HEIGHT - PrevCapsuleHeight;
		GetCapsuleComponent()->SetCapsuleHalfHeight(CROUCHING_CAPSULE_HALF_HEIGHT);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -CROUCHING_CAPSULE_HALF_HEIGHT));
		SetActorLocation(GetActorLocation() + FVector(0, 0, CapsuleHeightDifference));
		CameraBoom->SocketOffset = FVector(0, 0, 50);
		CameraBoom->TargetOffset = FVector(0, 0, 25);
		CameraBoom->TargetArmLength = 500;
	}
		break;

	case ECharacterState::CS_Crawling:
	{
		// set the collider to be a ball
		/*float PrevCapsuleHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
		float CapsuleHeightDifference = GetCapsuleComponent()->GetUnscaledCapsuleRadius() - PrevCapsuleHeight;
		GetCapsuleComponent()->SetCapsuleHalfHeight(GetCapsuleComponent()->GetUnscaledCapsuleRadius());
		GetMesh()->SetRelativeLocation(FVector(0, 0, -GetCapsuleComponent()->GetUnscaledCapsuleRadius()));
		SetActorLocation(GetActorLocation() + FVector(0, 0, CapsuleHeightDifference));*/
	}
		break;

	case ECharacterState::CS_LegOff:
	case ECharacterState::CS_LegOn:
	case ECharacterState::CS_ArmOff:
	case ECharacterState::CS_ArmOn:
	case ECharacterState::CS_StartCrawl:
	{
		float PrevCapsuleHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
		float CapsuleHeightDifference = CROUCHING_CAPSULE_HALF_HEIGHT - PrevCapsuleHeight;
		GetCapsuleComponent()->SetCapsuleHalfHeight(CROUCHING_CAPSULE_HALF_HEIGHT);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -CROUCHING_CAPSULE_HALF_HEIGHT));
		SetActorLocation(GetActorLocation() + FVector(0, 0, CapsuleHeightDifference));
	}
		break;

	}
	
	CurrentState = State;
}

void AErosCharacter::ExitState(ECharacterState State)
{
}

void AErosCharacter::TickState(float DeltaSeconds)
{
	switch (CurrentState)
	{

	case ECharacterState::CS_OnGround:

		if (GetCharacterMovement()->MovementMode == MOVE_Falling)
		{
			ChangeState(ECharacterState::CS_InAir);
		}
		else if (!HasProstheticOfType(EProstheticType::PT_Leg))
		{
			ChangeState(ECharacterState::CS_Crawling);
		}

		break;

	case ECharacterState::CS_InAir:

		if (GetCharacterMovement()->MovementMode == MOVE_Walking)
		{
			ChangeState(ECharacterState::CS_OnGround);
		}

		break;
		
	case ECharacterState::CS_Dropping:

		if (GetCharacterMovement()->MovementMode == MOVE_Walking)
		{
			ChangeState(ECharacterState::CS_OnGround);
		}

		break;

	}
}

void AErosCharacter::UpdateInteractableHighlights()
{
	NearestLeg = nullptr;
	NearestArm = nullptr;
	NearestButton = nullptr;

	for (int i = 0; i < NearbyInteractables.Num(); ++i)
	{
		AInteractableActor* TempActor = Cast<AInteractableActor>(NearbyInteractables[i]);

		// Piece of shit test. Can't find any better fix...
		if (TempActor == NewLegProstheticToAttach || TempActor == NewArmProstheticToAttach || TempActor == LegProstheticToRemove || TempActor == ArmProstheticToRemove)
		{
			TempActor->SetUIVisible(false);
			continue;
		}

		if (Cast<AUnattachedProsthetic>(TempActor))
		{
			// For each leg check if it is closer than the previous leg, and if it is then update the NearestLeg
			if (Cast<AUnattachedProsthetic>(TempActor)->GetType() == EProstheticType::PT_Leg)
			{
				if (NearestLeg)
				{
					if ((TempActor->GetActorLocation() - GetActorLocation()).Size() < (NearestLeg->GetActorLocation() - GetActorLocation()).Size())
					{
						NearestLeg = TempActor;
					}
				}
				else
				{
					NearestLeg = TempActor;
				}

			}
			// For each arm check if it is closer than the previous arm, and if it is then update the Nearestarm
			else if (Cast<AUnattachedProsthetic>(TempActor)->GetType() == EProstheticType::PT_Arm)
			{
				if (NearestArm)
				{
					if ((TempActor->GetActorLocation() - GetActorLocation()).Size() < (NearestArm->GetActorLocation() - GetActorLocation()).Size())
					{
						NearestArm = TempActor;
					}
				}
				else
				{
					NearestArm = TempActor;
				}
			}
		}
		else if (Cast<ASwitch>(TempActor) || Cast<AProstheticCabinet>(TempActor))
		{
			if (NearestButton)
			{
				if ((TempActor->GetActorLocation() - GetActorLocation()).Size() < (NearestButton->GetActorLocation() - GetActorLocation()).Size())
				{
					NearestButton = TempActor;
				}
			}
			else
			{
				NearestButton = TempActor;
			}
		}
	}

	// Set Nearest arm and leg UI to show and all other UI's to hide
	if (NearestLeg != PreviousNearestLeg)
	{
		if (PreviousNearestLeg)
		{
			PreviousNearestLeg->SetUIVisible(false);
		}
		if (NearestLeg)
		{
			NearestLeg->SetUIVisible(true);
		}
		PreviousNearestLeg = NearestLeg;
	}
	if (NearestArm != PreviousNearestArm)
	{
		if (PreviousNearestArm)
		{
			PreviousNearestArm->SetUIVisible(false);
		}
		if (NearestArm)
		{
			NearestArm->SetUIVisible(true);
		}
		PreviousNearestArm = NearestArm;
	}
	if (NearestButton != PreviousNearestButton)
	{
		if (PreviousNearestButton)
		{
			PreviousNearestButton->SetUIVisible(false);
		}
		if (NearestButton)
		{
			NearestButton->SetUIVisible(true);
		}
		PreviousNearestButton = NearestButton;
	}
}