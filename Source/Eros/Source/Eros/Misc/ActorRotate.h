#pragma once

#include "Components/ActorComponent.h"
#include "ActorRotate.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EROS_API UActorRotate : public UActorComponent
{
	GENERATED_BODY()

public:	
	UActorRotate();
		
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	UPROPERTY(EditAnywhere, Category = Rotation)
	FRotator Angle;
};
