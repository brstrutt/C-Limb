#pragma once

#include "GameFramework/Actor.h"
#include "ConveyorBelt.generated.h"

class AConveyorBeltSection;

UCLASS()
class EROS_API AConveyorBelt : public AActor
{
	GENERATED_BODY()
	
public:	

	AConveyorBelt();	

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	/* Enable/Disable tick to control if the conveyor is moving. */
	void ActivateConveyor(bool Active);

private:

	/* Collider to detect when a conveyor belt section has reached the end and needs placing back at the beginning. */
	UPROPERTY(EditAnywhere, Category = "Conveyor Belt")
	UBoxComponent* Collider;

	/* Start location of the conveyor belt. */
	UPROPERTY(EditAnywhere, Category = "Conveyor Belt")
	USceneComponent* Start;

	/* End location of the conveyor belt. */
	UPROPERTY(EditAnywhere, Category = "Conveyor Belt")
	USceneComponent* End;

	/* Single conveyor belt section to fill the between the start and end. */
	UPROPERTY(EditAnywhere, Category = "Conveyor Belt")
	TSubclassOf<AConveyorBeltSection> SectionPiece;

	/* How long is each section. */
	UPROPERTY(EditAnywhere, Category = "Conveyor Belt")
	unsigned PieceLength;

	/* How fast should the conveyor belt move in Unreal units per second.*/
	UPROPERTY(EditAnywhere, Category = "Conveyor Belt")
	float Speed;

	/* if true the conveyor will move when started. Else it will need to be told to start at some point. */
	UPROPERTY(EditAnywhere, Category = "Conveyor Belt")
	bool DefaultActive;

	/* All sections making up the conveyor belt. */
	TArray<AConveyorBeltSection*> ConveyorBelt;

	/* Direction from start to end. */
	FVector Direction;

	/* Handles replacing section pieces at the start. */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/* Move each section piece along. */
	void UpdatePieces(float DeltaSeconds);

	/* Move a piece back to the start. */
	void MoveToBeginning(AConveyorBeltSection& Section);
};
