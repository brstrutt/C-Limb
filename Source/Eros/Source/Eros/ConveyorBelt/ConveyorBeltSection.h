#pragma once

#include "GameFramework/Actor.h"
#include "ConveyorBeltSection.generated.h"

UCLASS()
class EROS_API AConveyorBeltSection : public AActor
{
	GENERATED_BODY()
	
public:	

	AConveyorBeltSection();	

private:

	UPROPERTY(EditAnywhere, Category = "Conveyor Belt")
	UStaticMeshComponent* Mesh;
};
