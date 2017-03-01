#include "Eros.h"

#include "../Character/ErosCharacter.h"
#include "../Character/Prosthetics/ProstheticSocket.h"
#include "../Character/Prosthetics/Prosthetic.h"
#include "AIDronePatrolPath.h"
#include "Character/ErosCharacter.h"
#include "DroneAICharacter.h"
#include "DroneAIController.h"
#include "EngineUtils.h"
#include "Sound/SoundCue.h"

#include <Components/SplineComponent.h>
#include <Components/DecalComponent.h>
#include <Perception/PawnSensingComponent.h>

ADroneAICharacter::ADroneAICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Initialise the sensing component
	EyesAndEars = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("Pawn Sensing Component"));

	// Initialise the default values of the decal
	OccludedRangeDisplay = CreateDefaultSubobject<UDecalComponent>(TEXT("Range of occluded hearing."));
	OccludedRangeDisplay->SetupAttachment(RootComponent);
	OccludedRangeDisplay->SetWorldRotation(FRotator(-90.0f, -90.0f, 0.0f));
	OccludedRangeDisplay->SetWorldScale3D(FVector(0.5f, 6.0f, 6.0f));
	OccludedRangeDisplay->SetRelativeLocation(FVector(0.0f, 0.0f, -130.0f));	
	ConstructorHelpers::FObjectFinder<UMaterialInterface> SoundRangeTexture(TEXT("Material'/Game/AI/Dron_Hear_Range.Dron_Hear_Range'"));	
	OccludedRangeDisplay->SetDecalMaterial(SoundRangeTexture.Object);

	// Initialise the default values of the point light
	MaxHearingRangeDisplay = CreateDefaultSubobject<UPointLightComponent>(TEXT("Range of un-occluded hearing."));
	MaxHearingRangeDisplay->SetupAttachment(RootComponent);
	MaxHearingRangeDisplay->SetLightColor(FLinearColor::Red);
	MaxHearingRangeDisplay->SetRelativeLocation(FVector(0.0f, 0.0f, 122.0f));
	MaxHearingRangeDisplay->SetIntensity(10.0f);
	MaxHearingRangeDisplay->SetAttenuationRadius(5000.0f);
	MaxHearingRangeDisplay->bUseInverseSquaredFalloff = false;
	MaxHearingRangeDisplay->SetLightFalloffExponent(0.0f);
	MaxHearingRangeDisplay->ShadowBias = 1.0f;
	MaxHearingRangeDisplay->ShadowSharpen = 1.0f;

	// Initialise the default values of the spotlight
	FOVSpotlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("FOV of the AI."));
	FOVSpotlight->SetupAttachment(RootComponent);
	FOVSpotlight->SetRelativeLocation(FVector(0.0f, 0.0f, 122.0f));
	FOVSpotlight->SetIntensity(10.0f);
	FOVSpotlight->bUseInverseSquaredFalloff = false;
	FOVSpotlight->SetLightFalloffExponent(0.0f);
	FOVSpotlight->ShadowBias = 1.0f;
	FOVSpotlight->ShadowSharpen = 1.0f;

	// Default movement speeds
	WalkingSpeed = 700.0f;
	RunningSpeed = 1000.0f;

	// Default Observe Duration
	ObserveDuration = 8.0f;

	AmbientAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("DroneAmbientAudioComponent"));
	AmbientAudioComp->SetupAttachment(RootComponent);
	AmbientAudioComp->bOverrideAttenuation = true;

	ReactionAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("DroneReactionAudioComponent"));
	ReactionAudioComp->SetupAttachment(RootComponent);
	ReactionAudioComp->bOverrideAttenuation = true;

	WarmupTime = 1.0f;
	CooldownTime = 1.0f;
}

void ADroneAICharacter::BeginPlay()
{
	Super::BeginPlay();

	// Set the PawnSensingComponent to call the NoiseDetected and CharacterSeen functions
	if (EyesAndEars)
	{
		EyesAndEars->OnHearNoise.AddDynamic(this, &ADroneAICharacter::NoiseDetected);
		EyesAndEars->OnSeePawn.AddDynamic(this, &ADroneAICharacter::CharacterSeen);

		EyesAndEars->Activate();

		// Set the spotlight's attributes according to the properties of the PawnSensingComponent
		FOVSpotlight->SetAttenuationRadius(EyesAndEars->SightRadius);
		FOVSpotlight->SetOuterConeAngle(EyesAndEars->GetPeripheralVisionAngle());
	}
	
	// Get the Player Character
	AErosCharacter* TargetPlayer = nullptr;
	for (TActorIterator<AErosCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		TargetPlayer = *ActorItr;
	}

	if (TargetPlayer)
	{
		// Update the Audio range every time a prosthetic is swapped, and once at the start
		TargetPlayer->OnProstheticSwapped.AddDynamic(this, &ADroneAICharacter::UpdateAudioRanges);

		UpdateAudioRanges(TargetPlayer);
	}

	// Set the audio range decal to use a dynamic material.
	UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(OccludedRangeDisplay->GetMaterial(0), this);
	OccludedRangeDisplay->SetMaterial(0, DynamicMaterial);

	SetAudioRangeColour(FLinearColor::Blue);

	AmbientAudioComp->SetSound(PatrollingWhir);
	AmbientAudioComp->Play();
}


void ADroneAICharacter::NoiseDetected(APawn* PawnInstigator, const FVector& Location, float Volume)
{
	float Distance = FVector::Dist(Location, GetActorLocation());

	UE_LOG(LogTemp, Warning, TEXT("Noise Heard. Distance: %f"), Distance);

	DroneController = Cast<ADroneAIController>(GetController());
	if (DroneController)
	{
		DroneController->PlayerHeard(PawnInstigator);
	}
}

void ADroneAICharacter::CharacterSeen(APawn* PawnInstigator)
{
	UE_LOG(LogTemp, Warning, TEXT("CharacterSeen."));

	DroneController = Cast<ADroneAIController>(GetController());
	if (DroneController)
	{
		DroneController->PlayerSeen(PawnInstigator);
	}
}

float ADroneAICharacter::GetSensingInterval()
{
	return EyesAndEars->SensingInterval;
}

int32 ADroneAICharacter::GetNumberOfPatrolPoints()
{
	// Return invalid value if there is no specified path
	if (!DronePatrolRoute) { return -1; }

	return DronePatrolRoute->PatrolRouteSpline->GetNumberOfSplinePoints();	
}

FVector ADroneAICharacter::GetLocationOfPatrolPoint(int32 PatrolPointIndex)
{
	// Return current position if this drone is not supposed to follow a path.
	if (!DronePatrolRoute) return GetActorLocation();

	return DronePatrolRoute->PatrolRouteSpline->GetLocationAtSplinePoint(PatrolPointIndex, ESplineCoordinateSpace::World);
}

int ADroneAICharacter::GetNearestSplinePoint()
{
	// Return invalid value if there is no specified path
	if (!DronePatrolRoute) return -1;

	return DronePatrolRoute->PatrolRouteSpline->FindInputKeyClosestToWorldLocation(GetActorLocation()) + 0.5f;
}

void ADroneAICharacter::SetAudioRangeColour(FLinearColor NewColour)
{
	Cast<UMaterialInstanceDynamic>(OccludedRangeDisplay->GetMaterial(0))->SetVectorParameterValue(FName(TEXT("BaseColour")), NewColour);
	MaxHearingRangeDisplay->SetLightColor(NewColour);
}

void ADroneAICharacter::SetWalking()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;
}

void ADroneAICharacter::SetRunning()
{
	GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
}

void ADroneAICharacter::PlayReactionSound(USoundCue* ReactionSoundClip)
{
	ReactionAudioComp->SetSound(ReactionSoundClip);
	ReactionAudioComp->Play();
}

void ADroneAICharacter::StopCurrentReactionSound()
{
	ReactionAudioComp->Stop();
}

void ADroneAICharacter::UpdateAudioRanges(AErosCharacter* PlayerCharacter)
{
	if (!PlayerCharacter) return;

	ScaleAudioRanges(PlayerCharacter->GetMaxVolume());
}

void ADroneAICharacter::ScaleAudioRanges(float Volume)
{
	// Use the hearing calculation from the PawnSensingComponent to calculate the max distance the player would be heard.
	float MaxDist = FMath::Sqrt(FMath::Square(EyesAndEars->HearingThreshold) * FMath::Square(Volume));

	// This sets the scale of the circle on the ground. Yes the Y and Z values control this. Don't question it. This code will hopefully be destroyed with fire anyway.
	OccludedRangeDisplay->SetWorldScale3D(FVector(OccludedRangeDisplay->GetComponentScale().X, MaxDist / 256.0f, MaxDist / 256.0f));

	// Set the range of the point light representing the unoccluded hearing range of the AI
	MaxDist = FMath::Sqrt(FMath::Square(EyesAndEars->LOSHearingThreshold) * FMath::Square(Volume));
	MaxHearingRangeDisplay->SetAttenuationRadius(MaxDist);
}

