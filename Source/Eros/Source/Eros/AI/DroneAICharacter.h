#pragma once

#include "GameFramework/Character.h"
#include "DroneAICharacter.generated.h"

class AAIDronePatrolPath;
class ADroneAIController;
class AErosCharacter;
class UBehaviorTree;
class UPawnSensingComponent;
class USoundCue;

/* This class is the physical AI character present in the world. It is controlled by the ADroneAIController class. */
UCLASS()
class EROS_API ADroneAICharacter : public ACharacter
{
	GENERATED_BODY()

public:

	ADroneAICharacter();

	virtual void BeginPlay() override;
	
	/* When hearing a noise, set the AIController's TargetLocation to the noise location. */
	UFUNCTION()
	void NoiseDetected(APawn* PawnInstigator, const FVector& Location, float Volume);

	/* When seeing the player, set the AIController's TargetLocation to the player's location. */
	UFUNCTION()
	void CharacterSeen(APawn* PawnInstigator);
	
	float GetSensingInterval();

	/* Get the number of points in the patrol path spline. */
	int32 GetNumberOfPatrolPoints();

	/* Get the location of a given point in the patrol path spline. */
	FVector GetLocationOfPatrolPoint(int32 PatrolPointIndex);

	/* Return the Index of the Spline Point nearest to this character. */
	int GetNearestSplinePoint();

	/* Change the colour of the Hearing Range point light and decal. */
	void SetAudioRangeColour(FLinearColor NewColour);

	/* Change the AI's walking speed between walking and running. */
	void SetWalking();

	void SetRunning();

	/* Play a provided sound clip without breaking ambient noise. */
	void PlayReactionSound(USoundCue* ReactionSoundClip);

	void StopCurrentReactionSound();

	FORCEINLINE float GetWarmupTime() const { return WarmupTime; }

	FORCEINLINE float GetCooldownTime() const { return CooldownTime; }

	FORCEINLINE float GetObserveDuration() const { return ObserveDuration; }

	FORCEINLINE UBehaviorTree* GetDroneBehaviour() { return DroneBehaviour; }
	
private:

	ADroneAIController* DroneController;

	/* The behaviour tree which controls this AI's behaviour. */
	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* DroneBehaviour;

	/* The component which allows the AI to see and hear the player. */
	UPROPERTY(VisibleAnywhere, Category = "AI")
	UPawnSensingComponent* EyesAndEars;
	
	/* The spline describing the AI's patrol route. */
	UPROPERTY(EditAnywhere, Category = "AI")
	AAIDronePatrolPath* DronePatrolRoute;

	/* The AI's walk and run speeds. */
	UPROPERTY(EditAnywhere, Category = "AI")
	float WalkingSpeed;

	UPROPERTY(EditAnywhere, Category = "AI")
	float RunningSpeed;

	/* The time the AI spends in the Observe state */
	UPROPERTY(EditAnywhere, Category = "AI")
	float ObserveDuration;

	/* The warmup and cooldown times the AI applies before/after chasing the player. */
	UPROPERTY(EditAnywhere, Category = "AI")
	float WarmupTime;

	UPROPERTY(EditAnywhere, Category = "AI")
	float CooldownTime;

	/* The decal that shows the AI's hearing range through walls. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	UDecalComponent* OccludedRangeDisplay;

	/* Point light showing the AI's un-occluded hearing range. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	UPointLightComponent* MaxHearingRangeDisplay;

	/* Spotlight showing the AI's FOV */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	USpotLightComponent* FOVSpotlight;	

	/* Component to play the Drone's ambient noises. */
	UAudioComponent* AmbientAudioComp;

	/* Component to play the Drone's reactions to the player. */
	UAudioComponent* ReactionAudioComp;

	/* Standard Drone Noise. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	USoundCue* PatrollingWhir;
	
	/* Update the Audio ranges using the provided ErosCharacter's Prosthetic information. */
	UFUNCTION()
	void UpdateAudioRanges(AErosCharacter* PlayerCharacter);

	/* Update the hearing ranges based on the volume of potential noises. */
	void ScaleAudioRanges(float Volume);
};
