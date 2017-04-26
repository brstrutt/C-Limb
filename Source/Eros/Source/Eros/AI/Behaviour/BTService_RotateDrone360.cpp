#include "Eros.h"

#include "../DroneAIController.h"
#include "../DroneAICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BTService_RotateDrone360.h"

UBTService_RotateDrone360::UBTService_RotateDrone360(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Rotate the character through 360 in the specified amount of time.";

	BlackboardKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RotateDrone360, BlackboardKey));
}

void UBTService_RotateDrone360::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!BlackboardKey.IsSet()) { return; }

	// Get the AIDroneCharacter to be rotated.
	if (ADroneAIController* DroneController = Cast<ADroneAIController>(OwnerComp.GetAIOwner()))
	{
		float RotationTime = DroneController->GetDroneBlackboardComp()->GetValueAsFloat(BlackboardKey.SelectedKeyName);

		// If the rotation duration is approximately zero then don't bother rotating
		if (!FMath::IsNearlyZero(RotationTime))
		{
			ADroneAICharacter* DroneCharacter = Cast<ADroneAICharacter>(OwnerComp.GetAIOwner()->GetPawn());

			FRotator NewRotation = DroneCharacter->GetActorRotation();
			NewRotation.Yaw += (360.0f / RotationTime) * DeltaSeconds;
			DroneCharacter->SetActorRotation(NewRotation);
		}
	}
}


