#include "Eros.h"

#include "DrawDebugHelpers.h"
#include "Grapple/GrappleAnchor.h"
#include "Grapple/GrappleAnchorReticle.h"
#include "Grapple/GrappleTarget.h"
#include "Prosthetic_GrappleArm.h"
#include "ProstheticSocket.h"
#include "CableComponent.h"

#define GRAPPLE_TARGET ECollisionChannel::ECC_GameTraceChannel6

AProsthetic_GrappleArm::AProsthetic_GrappleArm()
{
	PrimaryActorTick.bCanEverTick = true;

	Cable = CreateDefaultSubobject<UCableComponent>(TEXT("Cable"));
	Cable->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, false), FName("GrappleHookSocket"));
}

AProsthetic_GrappleArm::~AProsthetic_GrappleArm()
{
	UE_LOG(LogTemp, Warning, TEXT("DESTROYED"));

	if (InvisibleCableHead)
	{
		InvisibleCableHead->Destroy();
	}

	if (GrappleReticle)
	{
		GrappleReticle->Destroy();
	}
}

void AProsthetic_GrappleArm::SetMeshVisibility(bool bShow)
{
	Super::SetMeshVisibility(bShow);

	Cast<UStaticMeshComponent>(InvisibleCableHead->GetComponentByClass(UStaticMeshComponent::StaticClass()))->SetVisibility(bShow);
}

void AProsthetic_GrappleArm::BeginPlay()
{
	Super::BeginPlay();

	InvisibleCableHead = GetWorld()->SpawnActor<AActor>(InvisibleCableHeadTemplate.GetDefaultObject()->GetClass());

	GrappleReticle = GetWorld()->SpawnActor<AGrappleAnchorReticle>(GrappleReticleTemplate.GetDefaultObject()->GetClass());

	Cable->SetAttachEndTo(InvisibleCableHead, *InvisibleCableHead->GetRootComponent()->GetName());
	Cable->EndLocation = FVector::ZeroVector;
	Cable->SetVisibility(false);

	Timer = 0;
}

void AProsthetic_GrappleArm::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Keep the cable in the characters hand.
	if ((!CanHook && !IsHooking)) 
	{
		InvisibleCableHead->SetActorLocation(GetMesh()->GetSocketLocation(FName("GrappleHookSocket")));

		// Update the grapple reticle when we aren't grappling.
		UGrappleAnchor const* Anchor = GetClosestAnchor();

		if (IsValid(Anchor) && Character->GetState() == ECharacterState::CS_OnGround || Character->GetState() == ECharacterState::CS_Aiming)
		{
			GrappleReticle->SetTarget(Anchor);
		}
		else
		{
			GrappleReticle->Hide();
		}

		return;
	}

	if (CanHook)
	{
		// Direction from the cable to the anchor.
		FVector const DistanceFromTarget = TargetAnchor->GetComponentLocation() - InvisibleCableHead->GetActorLocation();
		FVector const Direction = (DistanceFromTarget).GetUnsafeNormal();

		FVector DistanceToMove = Direction * 1500.0f * DeltaSeconds;

		// Prevent the grapple from overshooting.
		if (DistanceToMove.SizeSquared() >= DistanceFromTarget.SizeSquared())
		{
			DistanceToMove = DistanceFromTarget;
		}

		InvisibleCableHead->AddActorWorldOffset(DistanceToMove);

		if (InvisibleCableHead->GetActorLocation() == TargetAnchor->GetComponentLocation())
		{
			FVector RotationVector = -TargetAnchor->GetForwardVector(); // TargetAnchor->GetComponentLocation() - Character->GetActorLocation();
			RotationVector.Z = 0;
			RotationVector.Normalize();

			// Pull the character slightly before the anchor.
			FVector const TargetLocation = TargetAnchor->GetComponentLocation() + (TargetAnchor->GetForwardVector() * 35.0f);

			FLatentActionInfo LatentInfo;
			LatentInfo.CallbackTarget = this;

			UKismetSystemLibrary::MoveComponentTo(GetSocket()->GetCharacter().GetRootComponent(), TargetLocation, RotationVector.Rotation(), true, false, GrappleSpeed, false, EMoveComponentAction::Type::Move, LatentInfo);
			IsHooking = true;
			CanHook = false;
			Timer = 0;
		}
	}
	else if (IsHooking)
	{
		Timer += DeltaSeconds;

		if (Timer >= GrappleSpeed)
		{
			Timer = 0;

			Character->ChangeState(ECharacterState::CS_InAir);

			IsHooking = false;
			Cable->SetVisibility(false);
		}
	}
}

void AProsthetic_GrappleArm::OnAttached()
{
	Super::OnAttached();

	Character = &GetSocket()->GetCharacter();
}

void AProsthetic_GrappleArm::PrimaryActionBegin()
{
	Super::PrimaryActionBegin();
	
	if (Character->GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Walking) { return; }

	Character->ChangeState(ECharacterState::CS_Aiming);
}

void AProsthetic_GrappleArm::PrimaryActionEnd()
{
	if (CanHook || IsHooking) { return; }
	
	if (IsValid((TargetAnchor = GetClosestAnchor())))
	{
		CanHook = true;
		Cable->SetVisibility(true);

		GrappleReticle->Hide();

		Character->ChangeState(ECharacterState::CS_Grappling);

		// Force the character to look in the direction they're aiming.
		FVector LookDirection = (TargetAnchor->GetComponentLocation() - Character->GetActorLocation()).GetUnsafeNormal();
		LookDirection.Z = 0.0f;
		
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;

		UKismetSystemLibrary::MoveComponentTo(Character->GetRootComponent(), Character->GetActorLocation(), FRotationMatrix::MakeFromX(LookDirection).Rotator(), false, false, 0.25f, false, EMoveComponentAction::Type::Move, LatentInfo);
	}
	else
	{
		Character->ChangeState(ECharacterState::CS_OnGround);
	}

	Super::PrimaryActionEnd();
}

UGrappleAnchor const* AProsthetic_GrappleArm::GetClosestAnchor() const
{
	UGrappleAnchor const* ClosestAnchor = nullptr;

	// We want to start the raycast from around the character position.
	float const CameraDistToCharacter = (Character->GetActorLocation() - Character->GetCameraLocation()).Size();

	FVector const StartLocation = Character->GetCameraLocation() + (Character->GetCameraForward() * CameraDistToCharacter);
	FVector const EndLoc = StartLocation + (Character->GetCameraForward() * RayMax);

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLoc, GRAPPLE_TARGET))
	{
		if (AGrappleTarget* Target = Cast<AGrappleTarget>(Hit.GetActor()))
		{
			ClosestAnchor = Target->GetClosestAnchor(Character->GetActorLocation(), Hit.Location);
		}
	}

	return ClosestAnchor;
}