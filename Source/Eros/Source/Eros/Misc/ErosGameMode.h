#pragma once

#include "GameFramework/GameModeBase.h"
#include "ErosGameMode.generated.h"

class AAudioManager;
class ADroneAICharacter;

/* Struct stores any information which needs to be saved when a checkpoint is encountered. */
USTRUCT()
struct FCheckpointData
{
	GENERATED_USTRUCT_BODY()	
	FVector Location;
	FRotator Rotation;
};

/* Store all information needed to reset a drone. */
USTRUCT()
struct FDroneRespawnState
{
	GENERATED_USTRUCT_BODY()

	ADroneAICharacter* TheDrone;
	FVector Location;
	FRotator Rotation;

	uint8 DroneBehaviourState;
	FVector TargetPointLocation;

	FBehaviourVariables DroneBehaviourVariables;
};

UCLASS(minimalapi)
class AErosGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AErosGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Hack)
	bool bDontRespawnPlayer = false;

	virtual void BeginPlay() override;

	/* Update information required to respawn the player given a new checkpoint. */
	void UpdateRespawnInformation(AActor* CheckpointHit, AErosCharacter* PlayerCharacter);
	

private:

	/* The position and rotation to respawn the player at if they die. */
	FCheckpointData LastCheckpoint;

	/* DEPRECATED! Drones will now reset to their starting locations, stored local to each drone.*/
	/* Store the state of all drones. They will revert to this state when the player dies. */
	//TArray<FDroneRespawnState> DroneRespawnStates;

	UClass* ArmClass;
	UClass* LegClass;

	/* Update the position and rotation of the respawn location. */
	void SetCheckpointData(FVector LastCheckpointLocation, FRotator LastCheckpointRotation);
	/* Loop through all drones in the world and store each one's current state. */
	void SetDroneStates();
	/* Pull the players current prosthetics and store their classes. */
	void SetRespawnProsthetics(AErosCharacter* PlayerCharacter);

	UFUNCTION()
	void OnProstheticSwapped(AErosCharacter* PlayerCharacter, AProsthetic* NewProsthetic);

	/* Called when the player is destroyed. */
	UFUNCTION()
	void OnPlayerDestroyed(AActor* DestroyedActor);
	/* Respawn the player with the specified arm and leg prosthetics. */
	void RespawnPlayer(UClass* ArmProsthetic, UClass* LegProsthetic);
	/* Move the drones back to their respawn state and reset their behaviour state. */
	void ResetDrones();
};



