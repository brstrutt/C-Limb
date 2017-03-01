#pragma once

#include "AIController.h"
#include "DroneAIController.generated.h"

class ADroneAICharacter;
class ATargetPoint;
class UBehaviorTreeComponent;
class UBlackboardComponent;
class USoundCue;

/* The different states the AI can be in. */
UENUM(BlueprintType)
enum class EAIBehaviourState : uint8
{
	BS_Patrol			UMETA(DisplayName = Patrol),
	BS_Observe			UMETA(DisplayName = Observe),
	BS_Investigate		UMETA(DisplayName = Investigate),
	BS_Chase			UMETA(DisplayName = Chase),
	BS_None				UMETA(DisplayName = None)
};

/* This class controls the behaviour of a corresponding ADroneAICharacter instance */
UCLASS()
class EROS_API ADroneAIController : public AAIController
{
	GENERATED_BODY()	

public:

	ADroneAIController();
	
	/* Initialise the AI behaviour for the provided pawn. */
	virtual void Possess(APawn* InPawn) override;

	/* Set PlayerSeenValue to maximum. */
	void PlayerSeen(APawn* SeenPawn);

	/* Switch to the Investigate State and set TargetLocation to the position of the provided Pawn. */
	void PlayerHeard(APawn* HeardPawn);
	
	/* Change state to the next most aggressive (e.g. Chase becomes Investigate). */
	void TargetPointReached();

	/* Returns the coordinates of the next point in the DroneAICharacter's spline. */
	FVector GetLocationOfNextPatrolPoint();

	FORCEINLINE UBlackboardComponent* GetDroneBlackboardComp() { return DroneBlackboardComp; }

	FORCEINLINE FName GetNextWaypointKey() { return NextWaypointKey; }

	FORCEINLINE FName GetTargetLocationKey() { return TargetLocationKey; }

	FORCEINLINE FName GetCurrentStateKey() { return CurrentStateKey; }

protected:
	/* Updates AI state. */
	virtual void Tick(float DeltaSeconds) override;

private:
	
	/* Store references to the AI's Blackboard and BehaviorTree */
	UPROPERTY(transient)
	UBlackboardComponent* DroneBlackboardComp;

	UPROPERTY(transient)
	UBehaviorTreeComponent* DroneBehaviourComp;

	/* Store references to the Blackboard's variables */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName NextWaypointKey;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetLocationKey;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetObjectKey;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName CurrentStateKey;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName ObserveWaitTimeKey;

	ATargetPoint* TargetLocation;

	/* The Drone character that this AIController has possessed. */
	ADroneAICharacter* ControlledDrone;

	/* Store Drone reaction clips. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	USoundCue* ChasingBeepLoop;

	/* Used to add warmup and cooldown to the AI seeing the player. */
	float AlertLevel;
	float WarmupTime;
	float CooldownTime;

	/* Keep track of the time between detections. */
	float TimeSinceLastSightDetection;

	/* Store if anything was detected this tick. */
	bool bPlayerSeenThisTick;
	bool bNoiseHeardThisTick;

	/* Store the distance to the player. The shorter the distance the faster the AI should spot you. */
	float DistToPlayer;

	/* Used to let the AI know when it should go down a state. */
	bool StateTaskComplete;

	/* Store the index of the next point in the spline. */
	int32 NextSplinePointIndex;

	/* Store the last place the AI was in the navmesh. */
	FVector LastNavmeshLocation;

	/* Warm up the alert level. */
	void TickAlertLevelUp(float DeltaSeconds);
	bool PlayerVisuallyDetected();

	/* Cool down the alert level. */
	void TickAlertLevelDown(float DeltaSeconds);
	bool PlayerVisuallyLost();

	/* Check for state changes based on the current State. */
	void ExecutePatrolTick(float DeltaSeconds);
	void ExecuteObserveTick(float DeltaSeconds);
	void ExecuteInvestigateTick(float DeltaSeconds);
	void ExecuteChaseTick(float DeltaSeconds);

	/* Enter State functions. */
	void EnterPatrolState();
	void EnterObserveState();
	void EnterInvestigateState();
	void EnterChaseState();

	/* Exit State functions. */
	void ExitPatrolState();
	void ExitObserveState();
	void ExitInvestigateState();
	void ExitChaseState();


};
