#include "Eros.h"

#include "../DroneAIController.h"
#include "BTTask_ExitState.h"

EBTNodeResult::Type UBTTask_ExitState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ADroneAIController* DroneController = Cast<ADroneAIController>(OwnerComp.GetAIOwner()))
	{
		//If there is a drone controller, reduce it's aggro
		DroneController->TargetPointReached();

		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}