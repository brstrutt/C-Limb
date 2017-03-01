#pragma once

#include <GameFramework/Actor.h>
#include "ProstheticDetector.generated.h"

class AProsthetic;
class AProstheticContainer;

UCLASS()
class EROS_API AProstheticDetector : public AActor
{
	GENERATED_BODY()
	
public:	

	AProstheticDetector();	

	virtual void BeginPlay() override;

private:

	bool bOpen;

	/* Metal detector/prosthetic detector mesh. */
	UPROPERTY(EditDefaultsOnly, Category = "Prosthetic Detection")
	UStaticMeshComponent* DetectorMesh;

	/* Collider to detect when the player is near. */
	UPROPERTY(EditDefaultsOnly, Category = "Prosthetic Detection")
	UStaticMeshComponent* DetectorCollider;

	/* Temporary mesh used to show when the door is opened or closed. */
	UPROPERTY(EditDefaultsOnly, Category = "Prosthetic Detection")
	UStaticMeshComponent* TemporaryDoor;

	/* Container which the player must place the required prosthetic in order to pass the detector. */
	UPROPERTY(EditAnywhere, Category = "Prosthetic Detection")
	AProstheticContainer* Container;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/* Open the detector door. */
	void Open();

	/* Close the detector door. */
	void Close();
};
