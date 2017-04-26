#pragma once

#include "AIController.h"
#include "DroneAIController.generated.h"

class ADroneAICharacter;
class ATargetPoint;
class UBehaviorTreeComponent;
class UBlackboardComponent;
class USoundCue;

/* Variables used to affect the drones behaviour and current state. */
USTRUCT()
struct FBehaviourVariables
{
	GENERATED_USTRUCT_BODY()
	/* Used to add warmup and cooldown to the AI seeing the player. */
	float AlertLevel;

	/* Keep track of the time between detections. */
	float TimeSinceLastSightDetection;

	/* Store if anything was detected this tick. */
	bool bPlayerSeenThisTick;
	//bool bNoiseHeardThisTick;

	/* Used to let the AI know when it should go down a state. */
	bool StateTaskComplete;
	
	/* Store the index of the next point in the spline. */
	int32 NextSplinePointIndex;

	/* Store the last place the AI was in the navmesh. */
	FVector LastNavmeshLocation;

	/* Store the distance to the player. The shorter the distance the faster the AI should spot you. */
	float DistToPlayer;
};

/* The different states the AI can be in. */
UENUM(BlueprintType)
enum class EAIBehaviourState : uint8
{
	BS_Patrol			UMETA(DisplayName = Patrol),
	BS_Observe			UMETA(DisplayName = Observe),
	BS_Alert			UMETA(DisplayName = Alert),
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

	/* Switch to the Heard State and set TargetLocation to the position of the provided Pawn. */
	void PlayerHeard(APawn* HeardPawn);
	
	/* Change state to the next most aggressive (e.g. Chase becomes Investigate). */
	void TargetPointReached();

	/* Returns the coordinates of the next point in the DroneAICharacter's spline. */
	FVector GetLocationOfNextPatrolPoint();
	
	void ResetDroneState();

	/* Get/Set the Drone's next destination on it's patrol route. */
	FORCEINLINE FBehaviourVariables GetDroneBehaviourVariables() { return DroneBehaviourVariables; };
	FORCEINLINE void SetDroneBehaviourVariables(FBehaviourVariables NewVars) { DroneBehaviourVariables = NewVars; };

	/* Get/Set the target location that the drone might be investigating. */
	FVector GetTargetPointLocation();
	void SetTargetPointLocation(FVector NewLocation);

	/* Get/Set the drone's current behaviour state. */
	uint8 GetBehaviourState();
	FORCEINLINE void SetBehaviourState(uint8 NewState) { SetBehaviourState((EAIBehaviourState)NewState); }
	void SetBehaviourState(EAIBehaviourState NewState);

	/* Get access to the Drone's blackboard and several of it's values. */
	FORCEINLINE UBlackboardComponent* GetDroneBlackboardComp() { return DroneBlackboardComp; }
	FORCEINLINE const FName& GetNextWaypointKey() const { return NextWaypointKey; }
	FORCEINLINE const FName& GetTargetLocationKey() const { return TargetLocationKey; }
	FORCEINLINE const FName& GetCurrentStateKey() const { return CurrentStateKey; }

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
	
	/* The Drone character that this AIController has possessed. */
	ADroneAICharacter* ControlledDrone;

	/* Store Drone reaction clips. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	USoundCue* ChasingBeepLoop;
	
	ATargetPoint* TargetLocation;

	/* Time it takes for the AI to notice the player. */
	float WarmupTime;
	/* Time it takes for the AI to stop chasing the player. */
	float CooldownTime;
	
	FBehaviourVariables DroneBehaviourVariables;

	FBehaviourVariables InitialDroneBehaviourVariables;

	/* Store the time since the navmesh was left. (needed for jumping drones. boing boing boing) */
	float TimeSinceLastOnNavmesh;

	/* Warm up the alert level. */
	void TickAlertLevelUp(float DeltaSeconds);
	bool PlayerVisuallyDetected();

	/* Cool down the alert level. */
	void TickAlertLevelDown(float DeltaSeconds);
	bool PlayerVisuallyLost();

	bool bNoiseHeardThisTick;

	/* Delegate to react to collision with the player. */
	UFUNCTION()
	void OnDroneCollision(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/* Check for state changes based on the current State. */
	void ExecutePatrolTick(float DeltaSeconds);
	void ExecuteObserveTick(float DeltaSeconds);
	void ExecuteAlertTick(float DeltaSeconds);
	void ExecuteInvestigateTick(float DeltaSeconds);
	void ExecuteChaseTick(float DeltaSeconds);

	/* Enter State functions. */
	void EnterState(EAIBehaviourState NewState);

	/* Exit State functions. */
	void ExitState(EAIBehaviourState OldState);
};
