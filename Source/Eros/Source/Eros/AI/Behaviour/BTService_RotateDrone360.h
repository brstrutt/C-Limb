#pragma once

#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_RotateDrone360.generated.h"

UCLASS()
class EROS_API UBTService_RotateDrone360 : public UBTService_BlackboardBase
{
	GENERATED_BODY()

	UBTService_RotateDrone360(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	/* Needed to compile. */
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) {}
	virtual void OnGameplayTaskDeactivated(UGameplayTask& Task) {}
	
public:

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
