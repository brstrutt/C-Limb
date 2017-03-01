#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_GetNextDestination.generated.h"

/* A Behavior Tree task to fetch the next waypoint in the AI's patrol route. */
UCLASS()
class EROS_API UBTTask_GetNextDestination : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

	/* Function required for compile to succeed. */
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override {}

public:

	/* Update OwnerComp's destination to the next waypoint in its path. */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};