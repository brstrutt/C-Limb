#include "Eros.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "DroneAIController.h"
#include "DroneAICharacter.h"
#include "Character/ErosCharacter.h"
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

	DroneBehaviourVariables.LastNavmeshLocation = FVector(0.0f, 0.0f, 0.0f);
	TimeSinceLastOnNavmesh = 0.0f;
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

		// Set the default Waypoint index to -1 (a check for -1 can be used to catch errors caused by there being no points in the spline)
		int32 NextSplinePointIndex = -1;		
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
		WarmupTime = ControlledDrone->GetWarmupTime();
		CooldownTime = ControlledDrone->GetCooldownTime();
		bNoiseHeardThisTick = false;
		
		// Create the AI's TargetLocation
		TargetLocation = Cast<ATargetPoint>(GetWorld()->SpawnActor(ATargetPoint::StaticClass()));
		DroneBlackboardComp->SetValueAsObject(TargetLocationKey, TargetLocation);

		// Store the Observe duration
		DroneBlackboardComp->SetValueAsFloat(ObserveWaitTimeKey, ControlledDrone->GetObserveDuration());
		
		ControlledDrone->GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ADroneAIController::OnDroneCollision);
		
		DroneBehaviourVariables = FBehaviourVariables{
			0.0f,
			0.0f,
			false,
			//false,
			false,
			NextSplinePointIndex,
			ControlledDrone->GetActorLocation(),
			0.0f };

		InitialDroneBehaviourVariables = DroneBehaviourVariables;
	}
}

void ADroneAIController::PlayerSeen(APawn* SeenPawn)
{
	DroneBehaviourVariables.bPlayerSeenThisTick = true;
	DroneBlackboardComp->SetValueAsObject(TargetObjectKey, SeenPawn);
	TargetLocation->SetActorLocation(SeenPawn->GetActorLocation());
	DroneBehaviourVariables.DistToPlayer = (SeenPawn->GetActorLocation() - ControlledDrone->GetActorLocation()).Size();
	DroneBehaviourVariables.TimeSinceLastSightDetection = 0.0f;
}

void ADroneAIController::PlayerHeard(APawn* HeardPawn)
{
	// Only register the noise if it occurred within the navmesh
	FNavLocation temp;
	bNoiseHeardThisTick = true;
	TargetLocation->SetActorLocation(HeardPawn->GetActorLocation());
}

void ADroneAIController::TargetPointReached()
{
	DroneBehaviourVariables.StateTaskComplete = true;
}

FVector ADroneAIController::GetLocationOfNextPatrolPoint()
{
	return ControlledDrone->GetLocationOfPatrolPoint(DroneBehaviourVariables.NextSplinePointIndex);
}

void ADroneAIController::ResetDroneState()
{
	SetDroneBehaviourVariables(InitialDroneBehaviourVariables);
	SetTargetPointLocation(FVector(0,0,0));
	SetBehaviourState(EAIBehaviourState::BS_None);
	TimeSinceLastOnNavmesh = 0.0f;
}

FVector ADroneAIController::GetTargetPointLocation()
{
	return TargetLocation->GetActorLocation();
}

void ADroneAIController::SetTargetPointLocation(FVector NewLocation)
{
	TargetLocation->SetActorLocation(NewLocation);
}

uint8 ADroneAIController::GetBehaviourState()
{
	return DroneBlackboardComp->GetValueAsEnum(CurrentStateKey);
}

void ADroneAIController::SetBehaviourState(EAIBehaviourState NewState)
{
	ExitState((EAIBehaviourState)DroneBlackboardComp->GetValueAsEnum(CurrentStateKey));
	EnterState(NewState);
}

void ADroneAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	switch ((EAIBehaviourState)DroneBlackboardComp->GetValueAsEnum(CurrentStateKey))
	{
		case EAIBehaviourState::BS_Patrol:
			TickAlertLevelUp(DeltaSeconds);
			// Update the colour of the AI's detection range to match the level of alertness
			ControlledDrone->SetAudioRangeColour(FLinearColor::White - FLinearColor(0.0f, DroneBehaviourVariables.AlertLevel / WarmupTime, DroneBehaviourVariables.AlertLevel / WarmupTime));
			ExecutePatrolTick(DeltaSeconds);
			break;
		case EAIBehaviourState::BS_Observe:
			TickAlertLevelUp(DeltaSeconds);
			// Update the colour of the AI's detection range to match the level of alertness
			ControlledDrone->SetAudioRangeColour(FLinearColor::White -  FLinearColor(0.0f, DroneBehaviourVariables.AlertLevel / WarmupTime, DroneBehaviourVariables.AlertLevel / WarmupTime));
			ExecuteObserveTick(DeltaSeconds);
			break;
		case EAIBehaviourState::BS_Alert:
			TickAlertLevelUp(DeltaSeconds);
			// Update the colour of the AI's detection range to match the level of alertness
			ControlledDrone->SetAudioRangeColour(FLinearColor::White - FLinearColor(0.0f, DroneBehaviourVariables.AlertLevel / WarmupTime, DroneBehaviourVariables.AlertLevel / WarmupTime));
			ExecuteAlertTick(DeltaSeconds);
			break;
		case EAIBehaviourState::BS_Investigate:
			TickAlertLevelUp(DeltaSeconds);
			// Update the colour of the AI's detection range to match the level of alertness
			ControlledDrone->SetAudioRangeColour(FLinearColor::Yellow - FLinearColor(0.0f , DroneBehaviourVariables.AlertLevel / WarmupTime, DroneBehaviourVariables.AlertLevel / WarmupTime));
			ExecuteInvestigateTick(DeltaSeconds);
			break;
		case EAIBehaviourState::BS_Chase :
			TickAlertLevelDown(DeltaSeconds);
			ExecuteChaseTick(DeltaSeconds);
			break;
		case EAIBehaviourState::BS_None:
			SetBehaviourState(EAIBehaviourState::BS_Patrol);
			break;
	}

	// Reset detections in preparation for the next tick.
	DroneBehaviourVariables.bPlayerSeenThisTick = false;
	bNoiseHeardThisTick = false;

	DroneBehaviourVariables.TimeSinceLastSightDetection += DeltaSeconds;


	// Revert the AI's position if it left the navmesh
	FNavLocation temp;
	if (UNavigationSystem::GetCurrent(GetWorld())->ProjectPointToNavigation(ControlledDrone->GetActorLocation(), temp))
	{
		DroneBehaviourVariables.LastNavmeshLocation = ControlledDrone->GetActorLocation();
		TimeSinceLastOnNavmesh = 0.0f;
	}
	else
	{
		TimeSinceLastOnNavmesh += DeltaSeconds;

		if(TimeSinceLastOnNavmesh > 1.0f) ControlledDrone->SetActorLocation(DroneBehaviourVariables.LastNavmeshLocation);
	}
}

void ADroneAIController::TickAlertLevelUp(float DeltaSeconds)
{
	if ((DroneBehaviourVariables.TimeSinceLastSightDetection < ControlledDrone->GetSensingInterval() * 1.1f) && (DroneBehaviourVariables.DistToPlayer != 0.0f))
	{
		// Increase alert level based on how close the player was when it was seen
		DroneBehaviourVariables.AlertLevel += DeltaSeconds * (2000.0f / DroneBehaviourVariables.DistToPlayer);

		if (DroneBehaviourVariables.AlertLevel > WarmupTime)
		{
			DroneBehaviourVariables.AlertLevel = WarmupTime;
		}
	}
	else
	{
		DroneBehaviourVariables.AlertLevel -= DeltaSeconds;

		if (DroneBehaviourVariables.AlertLevel < 0.0f)
		{ 
			DroneBehaviourVariables.AlertLevel = 0.0f;
		}
	}
}

bool ADroneAIController::PlayerVisuallyDetected()
{
	return DroneBehaviourVariables.AlertLevel - WarmupTime >= -0.001f;
}

void ADroneAIController::TickAlertLevelDown(float DeltaSeconds)
{
	DroneBehaviourVariables.AlertLevel -= DeltaSeconds;

	if (DroneBehaviourVariables.bPlayerSeenThisTick)
	{
		DroneBehaviourVariables.AlertLevel = WarmupTime;
	}
}

bool ADroneAIController::PlayerVisuallyLost()
{
	return DroneBehaviourVariables.AlertLevel - (WarmupTime - CooldownTime) < 0.001;
}

void ADroneAIController::OnDroneCollision(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	/*if (OtherActor && OtherActor->GetClass()->IsChildOf(AErosCharacter::StaticClass()))
	{
		bPlayerSeenThisTick = false;
		//bPlayerHeardThisTick = false;
		AlertLevel = 0.0f;
		switch ((EAIBehaviourState)DroneBlackboardComp->GetValueAsEnum(CurrentStateKey))
		{
		case EAIBehaviourState::BS_Patrol:
			break;
		case EAIBehaviourState::BS_Observe:
			break;
		case EAIBehaviourState::BS_Investigate:
			ExitInvestigateState();
			EnterPatrolState();
			break;
		case EAIBehaviourState::BS_Chase:
			ExitChaseState();
			EnterPatrolState();
			break;
		case EAIBehaviourState::BS_None:
			EnterPatrolState();
			break;
		}
	}*/
}

void ADroneAIController::ExecutePatrolTick(float DeltaSeconds)
{
	//If the player has been in sight recently switch to chase
	if (PlayerVisuallyDetected())
	{
		SetBehaviourState(EAIBehaviourState::BS_Chase);
	}
	else if (bNoiseHeardThisTick || DroneBehaviourVariables.bPlayerSeenThisTick)
	{
		SetBehaviourState(EAIBehaviourState::BS_Alert);
	}
	//If the next waypoint has been reached then pause to observe
	else if (DroneBehaviourVariables.StateTaskComplete)
	{
		SetBehaviourState(EAIBehaviourState::BS_Observe);

		// Update the destination point
		if (DroneBehaviourVariables.NextSplinePointIndex != -1)
		{
			++DroneBehaviourVariables.NextSplinePointIndex;
			if(DroneBehaviourVariables.NextSplinePointIndex >= ControlledDrone->GetNumberOfPatrolPoints()) DroneBehaviourVariables.NextSplinePointIndex = 0;
		}
	}
}

void ADroneAIController::ExecuteObserveTick(float DeltaSeconds)
{
	//If the player has been in sight recently switch to chase
	if (PlayerVisuallyDetected())
	{
		SetBehaviourState(EAIBehaviourState::BS_Chase);
	}
	else if (bNoiseHeardThisTick)
	{
		SetBehaviourState(EAIBehaviourState::BS_Alert);
	}
	//If the Observe phase is complete switch back to patrol
	else if (DroneBehaviourVariables.StateTaskComplete)
	{
		SetBehaviourState(EAIBehaviourState::BS_Patrol);
	}
}

void ADroneAIController::ExecuteAlertTick(float DeltaSeconds)
{
	if (PlayerVisuallyDetected())
	{
		SetBehaviourState(EAIBehaviourState::BS_Chase);
	}
	else if (DroneBehaviourVariables.StateTaskComplete)
	{
		SetBehaviourState(EAIBehaviourState::BS_Patrol);
	}
}

void ADroneAIController::ExecuteInvestigateTick(float DeltaSeconds)
{
	//If the player has been in sight recently switch to chase
	if (PlayerVisuallyDetected())
	{
		SetBehaviourState(EAIBehaviourState::BS_Chase);
	}
	//If the Location to investigate has been reached switch back to Patrolling
	else if (DroneBehaviourVariables.StateTaskComplete)
	{
		SetBehaviourState(EAIBehaviourState::BS_Observe);
	}
}

void ADroneAIController::ExecuteChaseTick(float DeltaSeconds)
{
	if (PlayerVisuallyLost())
	{
		DroneBehaviourVariables.AlertLevel = 0.0f;
		
		// Set the TargetLocation for the investigate state
		AActor* CurrentTarget = Cast<AActor>(DroneBlackboardComp->GetValueAsObject(TargetObjectKey));
		if (CurrentTarget)
		{
			TargetLocation->SetActorLocation(CurrentTarget->GetActorLocation());
			SetBehaviourState(EAIBehaviourState::BS_Investigate);
		}
		else
		{
			SetBehaviourState(EAIBehaviourState::BS_Observe);
		}
	}
}

void ADroneAIController::EnterState(EAIBehaviourState NewState)
{
	switch (NewState)
	{
		case EAIBehaviourState::BS_Patrol:
			ControlledDrone->SetWalking();
			DroneBlackboardComp->SetValueAsEnum(CurrentStateKey, (uint8)EAIBehaviourState::BS_Patrol);
			ControlledDrone->SetAudioRangeColour(FLinearColor::White);
			DroneBehaviourVariables.StateTaskComplete = false;
			break;
		case EAIBehaviourState::BS_Observe:
			DroneBlackboardComp->SetValueAsEnum(CurrentStateKey, (uint8)EAIBehaviourState::BS_Observe);
			ControlledDrone->SetAudioRangeColour(FLinearColor::White);
			DroneBehaviourVariables.StateTaskComplete = false;
			break;
		case EAIBehaviourState::BS_Alert:
			DroneBlackboardComp->SetValueAsEnum(CurrentStateKey, (uint8)EAIBehaviourState::BS_Alert);
			DroneBehaviourVariables.StateTaskComplete = false;
			break;
		case EAIBehaviourState::BS_Investigate:
			ControlledDrone->SetRunning();
			DroneBlackboardComp->SetValueAsEnum(CurrentStateKey, (uint8)EAIBehaviourState::BS_Investigate);
			ControlledDrone->SetAudioRangeColour(FLinearColor::Yellow);
			DroneBehaviourVariables.StateTaskComplete = false;
			break;
		case EAIBehaviourState::BS_Chase:
			ControlledDrone->SetRunning();
			DroneBlackboardComp->SetValueAsEnum(CurrentStateKey, (uint8)EAIBehaviourState::BS_Chase);
			ControlledDrone->SetAudioRangeColour(FLinearColor::Red);
			DroneBehaviourVariables.StateTaskComplete = false;
			ControlledDrone->PlayReactionSound(ChasingBeepLoop);
			break;
		case EAIBehaviourState::BS_None:
			DroneBlackboardComp->SetValueAsEnum(CurrentStateKey, (uint8)EAIBehaviourState::BS_None);
			DroneBehaviourVariables.StateTaskComplete = false;
			break;
	}
}

void ADroneAIController::ExitState(EAIBehaviourState OldState)
{
	switch (OldState)
	{
	case EAIBehaviourState::BS_Patrol:
		break;
	case EAIBehaviourState::BS_Observe:
		break;
	case EAIBehaviourState::BS_Alert:
		break;
	case EAIBehaviourState::BS_Investigate:
		break;
	case EAIBehaviourState::BS_Chase:
		ControlledDrone->StopCurrentReactionSound();
		break;
	}
}