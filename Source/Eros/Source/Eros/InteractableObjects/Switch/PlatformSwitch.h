#pragma once

#include "Switch.h"
#include "PlatformSwitch.generated.h"

UCLASS(Blueprintable)
class EROS_API APlatformSwitch : public ASwitch
{
	GENERATED_BODY()

public:

	APlatformSwitch();

	virtual void BeginPlay() override;
	
protected:

	virtual void Activate() override;

	virtual void Swap() override;

	virtual void Tick(float DeltaSeconds) override;

private:

	/* Speed, in Unreal units, which the platform should move at. */
	UPROPERTY(EditAnywhere, Category = Platform)
	float MovementSpeed;

	/* The actor in the scene to move. */
	UPROPERTY(EditAnywhere, Category = Platform)
	AActor* Platform;

	/* Mesh for the switch. */
	UPROPERTY(EditDefaultsOnly, Category = Platform)
	UStaticMeshComponent* Mesh;

	/* Distance from the origin to move. Used to calculate the destination position. */
	UPROPERTY(EditAnywhere, Category = Platform)
	FVector MovementDistance;

	/* Origin of the platform to move. */
	FVector Origin;

	/* Destination of the platform to move.*/
	FVector Destination;
};
