#include "Eros.h"

#include "../ErosCharacter.h"
#include "DrawDebugHelpers.h"
#include "Prosthetic_GrappleArm.h"
#include "ProstheticSocket.h"

AProsthetic_GrappleArm::AProsthetic_GrappleArm()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AProsthetic_GrappleArm::PrimaryActionBegin()
{
	AErosCharacter& Character = GetSocket()->GetCharacter();

	if (Character.GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Falling) return;

	FVector StartLoc = Character.GetCameraLocation() + (Character.GetCameraForward() * 100.0f);

	FVector EndLoc = StartLoc + (Character.GetCameraForward() * RayLength);

	FCollisionQueryParams CollisionParams = FCollisionQueryParams::DefaultQueryParam;
	CollisionParams.AddIgnoredActor(&Character);

	FCollisionObjectQueryParams ObjectParams = FCollisionObjectQueryParams::DefaultObjectQueryParam;

	FHitResult Hit;

	GetWorld()->LineTraceSingleByObjectType(Hit, StartLoc, EndLoc, ObjectParams, CollisionParams);

	/*Check the First Hit actor for its components tags and if they have a 'Hookable' tag then hook them to the center of the object*/
	/*Can probably be reordered to remove the negative check*/
	if (Hit.GetActor())
	{
		if (!Hit.GetComponent()->ComponentHasTag("Hookable"))
		{
			UE_LOG(LogTemp, Warning, TEXT("NOT HOOKABLE"));
			DrawDebugLine(GetWorld(), StartLoc, EndLoc, FColor::Green, false, 1.0f, 0, 2.0f);
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("HOOKABLE"));
		DrawDebugLine(GetWorld(), StartLoc, Hit.GetActor()->GetActorLocation(), FColor::Green, false, 1.0f, 0, 2.0f);
		
		//Character.LaunchCharacter((Hit.GetActor()->GetActorLocation() - Character.GetActorLocation()) * 1.5f, false, false);

		FVector RotationVector = Hit.GetActor()->GetActorLocation() - Character.GetActorLocation();
		RotationVector.Z = 0;
		RotationVector.Normalize();
		
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;

		UKismetSystemLibrary::MoveComponentTo(Character.GetRootComponent(), Hit.Location, RotationVector.Rotation(), true, false, GrappleSpeed, false, EMoveComponentAction::Type::Move, LatentInfo);
	}
}