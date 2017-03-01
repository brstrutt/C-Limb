#include "Eros.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "DroneAIController.h"
#include "DroneAICharacter.h"
#include "EngineUtils.h"
#include "Sound/SoundCue.h"

#include <Classes/Engine/TargetPoint.h>
#include <Components/SplineComponent.h>

ADroneAIController::ADroneAIController()
{
	// Initialise the blackboard and behaviour tree
	DroneBlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("DroneBlackboardComponent"));
	DroneBehaviourComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("DroneBehaviourTreeComponent"));

	// Initialise the blackboard variable references
	NextWaypointKey = "NextWaypoint";
	TargetLocationKey = "TargetLocation";
	TargetObjectKey = "TargetObject";
	CurrentStateKey = "AIState";
	ObserveWaitTimeKey = "ObserveWaitTime";

	LastNavmeshLocation = FVector(0.0f, 0.0f, 0.0f);
}

void ADroneAIController::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);

	// Obtain the DroneAICharacter being possessed
	ControlledDrone = Cast<ADroneAICharacter>(InPawn);
	if (ControlledDrone && ControlledDrone->GetDroneBehaviour())
	{
		if (ControlledDrone->GetDroneBehaviour()->BlackboardAsset)
		{
			DroneBlackboardComp->InitializeBlackboard(*(ControlledDrone->GetDroneBehaviour()->BlackboardAsset));
		}
		// Start the behaviour tree
		DroneBehaviourComp->StartTree(*ControlledDrone->GetDroneBehaviour());

		// Initialise AI state to nothing
		DroneBlackboardComp->SetValueAsEnum(CurrentStateKey, (uint8)EAIBehaviourState::BS_None);

		// Set the defaul Waypoint index to -1 (a check for -1 can be used to catch errors caused by there being no points in the spline)
		NextSplinePointIndex = -1;
		
		// Set the first destination
		if (ControlledDrone->GetNumberOfPatrolPoints() > 0)
		{
			// Get the nearest point on the spline to the character
			NextSplinePointIndex = ControlledDrone->GetNearestSplinePoint();

			if (NextSplinePointIndex != -1)
			{
				// Set the NEXT point as the first destination
				++NextSplinePointIndex;
				if (NextSplinePointIndex >= ControlledDrone->GetNumberOfPatrolPoints()) NextSplinePointIndex = 0;

				DroneBlackboardComp->SetValueAsVector(NextWaypointKey, ControlledDrone->GetLocationOfPatrolPoint(NextSplinePointIndex));
			}
		}

		// Default Alert Values
		AlertLevel = 0.0f;
		WarmupTime = ControlledDrone->GetWarmupTime();
		CooldownTime = ControlledDrone->GetCooldownTime();

		TimeSinceLastSightDetection = 0.0f;
		bPlayerSeenThisTick = false;
		bNoiseHeardThisTick = false;
		DistToPlayer = 0.0f;

		// Create the AI's TargetLocation
		TargetLocation = Cast<ATargetPoint>(GetWorld()->SpawnActor(ATargetPoint::StaticClass()));
		DroneBlackboardComp->SetValueAsObject(TargetLocationKey, TargetLocation);

		// Store the Observe duration
		DroneBlackboardComp->SetValueAsFloat(ObserveWaitTimeKey, ControlledDrone->GetObserveDuration());
	}
}

void ADroneAIController::PlayerSeen(APawn* SeenPawn)
{
	bPlayerSeenThisTick = true;
	DroneBlackboardComp->SetValueAsObject(TargetObjectKey, SeenPawn);
	TargetLocation->SetActorLocation(SeenPawn->GetActorLocation());

	DistToPlayer = (SeenPawn->GetActorLocation() - ControlledDrone->GetActorLocation()).Size();
	TimeSinceLastSightDetection = 0.0f;
}

void ADroneAIController::PlayerHeard(APawn* HeardPawn)
{
	// Only register the noise if it occurred within the navmesh
	FNavLocation temp;
	if (UNavigationSystem::GetCurrent(GetWorld())->ProjectPointToNavigation(HeardPawn->GetActorLocation(), temp))
	{
		bNoiseHeardThisTick = true;
		TargetLocation->SetActorLocation(HeardPawn->GetActorLocation());
	}
}

void ADroneAIController::TargetPointReached()
{
	StateTaskComplete = true;
}

FVector ADroneAIController::GetLocationOfNextPatrolPoint()
{
	return ControlledDrone->GetLocationOfPatrolPoint(NextSplinePointIndex);
}

void ADroneAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	switch ((EAIBehaviourState)DroneBlackboardComp->GetValueAsEnum(CurrentStateKey))
	{
		case EAIBehaviourState::BS_Patrol:
			TickAlertLevelUp(DeltaSeconds);
			// Update the colour of the AI's detection range to match the level of alertness
			ControlledDrone->SetAudioRangeColour(FLinearColor(AlertLevel / WarmupTime, 0.0f, 1.0f - (AlertLevel / WarmupTime)));
			ExecutePatrolTick(DeltaSeconds);
			break;
		case EAIBehaviourState::BS_Observe:
			TickAlertLevelUp(DeltaSeconds);
			// Update the colour of the AI's detection range to match the level of alertness
			ControlledDrone->SetAudioRangeColour(FLinearColor(AlertLevel / WarmupTime, 0.0f, 1.0f - (AlertLevel / WarmupTime)));
			ExecuteObserveTick(DeltaSeconds);
			break;
		case EAIBehaviourState::BS_Investigate:
			TickAlertLevelUp(DeltaSeconds);
			// Update the colour of the AI's detection range to match the level of alertness
			ControlledDrone->SetAudioRangeColour(FLinearColor(1.0f , 1.0f - (AlertLevel / WarmupTime), 0.0f));
			ExecuteInvestigateTick(DeltaSeconds);
			break;
		case EAIBehaviourState::BS_Chase :
			TickAlertLevelDown(DeltaSeconds);
			ExecuteChaseTick(DeltaSeconds);
			break;
		case EAIBehaviourState::BS_None:
			EnterPatrolState();
			break;
	}

	// Reset detections in preparation for the next tick.
	bPlayerSeenThisTick = false;
	bNoiseHeardThisTick = false;

	TimeSinceLastSightDetection += DeltaSeconds;


	// Revert the AI's position if it left the navmesh
	FNavLocation temp;
	if (UNavigationSystem::GetCurrent(GetWorld())->ProjectPointToNavigation(ControlledDrone->GetActorLocation(), temp))
	{
		LastNavmeshLocation = ControlledDrone->GetActorLocation();
	}
	else
	{
		ControlledDrone->SetActorLocation(LastNavmeshLocation);
	}
}

void ADroneAIController::TickAlertLevelUp(float DeltaSeconds)
{
	if ((TimeSinceLastSightDetection < ControlledDrone->GetSensingInterval() * 1.1f) && (DistToPlayer != 0.0f))
	{
		// Increase alert level based on how close the player was when it was seen
		AlertLevel += DeltaSeconds * (2000.0f / DistToPlayer);

		if (AlertLevel > WarmupTime)
		{
			AlertLevel = WarmupTime;
		}
	}
	else
	{
		AlertLevel -= DeltaSeconds;

		if (AlertLevel < 0.0f) 
		{ 
			AlertLevel = 0.0f; 
		}
	}
}

bool ADroneAIController::PlayerVisuallyDetected()
{
	// (AlertLevel >= WarmupTime) would work if they weren't floats.
	return AlertLevel - WarmupTime >= -0.001f;
}

void ADroneAIController::TickAlertLevelDown(float DeltaSeconds)
{
	AlertLevel -= DeltaSeconds; 

	if (bPlayerSeenThisTick)
	{
		AlertLevel = WarmupTime;
	}
}

bool ADroneAIController::PlayerVisuallyLost()
{
	// (AlertLevel < WarmupTime - CooldownTime) would work if they weren't floats
	return AlertLevel - (WarmupTime - CooldownTime) < 0.001;
}

void ADroneAIController::ExecutePatrolTick(float DeltaSeconds)
{
	//If the player has been in sight recently switch to chase
	if (PlayerVisuallyDetected())
	{
		ExitPatrolState();
		EnterChaseState();
	}
	else if (bNoiseHeardThisTick)
	{
		ExitPatrolState();
		EnterInvestigateState();
	}
	//If the next waypoint has been reached then pause to observe
	else if (StateTaskComplete)
	{
		ExitPatrolState();
		EnterObserveState();

		// Update the destination point
		if (NextSplinePointIndex != -1)
		{
			++NextSplinePointIndex;
			ADroneAICharacter* Drone = Cast<ADroneAICharacter>(GetPawn());
			if(NextSplinePointIndex >= Drone->GetNumberOfPatrolPoints()) NextSplinePointIndex = 0;
		}
	}
}

void ADroneAIController::ExecuteObserveTick(float DeltaSeconds)
{
	//If the player has been in sight recently switch to chase
	if (PlayerVisuallyDetected())
	{
		ExitObserveState();
		EnterChaseState();
	}
	else if (bNoiseHeardThisTick)
	{
		ExitObserveState();
		EnterInvestigateState();
	}
	//If the Observe phase is complete switch back to patrol
	else if (StateTaskComplete)
	{
		ExitObserveState();
		EnterPatrolState();
	}
}

void ADroneAIController::ExecuteInvestigateTick(float DeltaSeconds)
{
	//If the player has been in sight recently switch to chase
	if (PlayerVisuallyDetected())
	{
		ExitInvestigateState();
		EnterChaseState();
	}
	//If the Location to investigate has been reached switch back to Patrolling
	else if (StateTaskComplete)
	{
		ExitInvestigateState();
		EnterObserveState();
	}
}

void ADroneAIController::ExecuteChaseTick(float DeltaSeconds)
{
	if (PlayerVisuallyLost())
	{
		AlertLevel = 0.0f;
		ExitChaseState();
		
		// Set the TargetLocation for the investigate state
		AActor* CurrentTarget = Cast<AActor>(DroneBlackboardComp->GetValueAsObject(TargetObjectKey));
		TargetLocation->SetActorLocation(CurrentTarget->GetActorLocation());
		EnterInvestigateState();
	}
}


void ADroneAIController::EnterPatrolState()
{
	ControlledDrone->SetWalking();
	DroneBlackboardComp->SetValueAsEnum(CurrentStateKey, (uint8)EAIBehaviourState::BS_Patrol);
	ControlledDrone->SetAudioRangeColour(FLinearColor::Blue);
	StateTaskComplete = false;
}

void ADroneAIController::EnterObserveState()
{
	DroneBlackboardComp->SetValueAsEnum(CurrentStateKey, (uint8)EAIBehaviourState::BS_Observe);
	StateTaskComplete = false;
}

void ADroneAIController::EnterInvestigateState()
{
	ControlledDrone->SetRunning();
	DroneBlackboardComp->SetValueAsEnum(CurrentStateKey, (uint8)EAIBehaviourState::BS_Investigate);
	ControlledDrone->SetAudioRangeColour(FLinearColor::Yellow);
	StateTaskComplete = false;
}

void ADroneAIController::EnterChaseState()
{
	ControlledDrone->SetRunning();
	DroneBlackboardComp->SetValueAsEnum(CurrentStateKey, (uint8)EAIBehaviourState::BS_Chase);
	ControlledDrone->SetAudioRangeColour(FLinearColor::Red);
	StateTaskComplete = false;
	ControlledDrone->PlayReactionSound(ChasingBeepLoop);
}

void ADroneAIController::ExitPatrolState()
{
}

void ADroneAIController::ExitObserveState()
{
}

void ADroneAIController::ExitInvestigateState()
{
}

void ADroneAIController::ExitChaseState()
{
	ControlledDrone->StopCurrentReactionSound();
}
