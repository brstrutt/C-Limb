#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ExitState.generated.h"

UCLASS()
class EROS_API UBTTask_ExitState : public UBTTaskNode
{
	GENERATED_BODY()

	/* Function required for compile to succeed. */
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) {}

public:
	/* Reduce the AI's aggro state by one level. */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
