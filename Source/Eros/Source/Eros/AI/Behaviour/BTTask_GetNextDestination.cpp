#include "Eros.h"

#include "../DroneAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BTTask_GetNextDestination.h"

EBTNodeResult::Type UBTTask_GetNextDestination::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ADroneAIController* DroneController = Cast<ADroneAIController>(OwnerComp.GetAIOwner()))
	{		
		//Get the current destination from the blackboard
		UBlackboardComponent* BlackboardComp = DroneController->GetDroneBlackboardComp();	
		
		BlackboardComp->SetValueAsVector("NextWaypoint", DroneController->GetLocationOfNextPatrolPoint());
		
		return EBTNodeResult::Succeeded;
		
	}
	return EBTNodeResult::Failed;
}