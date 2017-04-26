#include "Eros.h"
#include "SpacedSoundCue.h"
#include "Sound/SoundCue.h"

ASpacedSoundCue::ASpacedSoundCue()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
}

void ASpacedSoundCue::BeginPlay()
{
	Super::BeginPlay();

	if (bStartEnabled)
	{
		Enable();
	}
	else
	{
		Disable();
	}
}

void ASpacedSoundCue::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!AudioComponent->IsPlaying())
	{
		if ((Timer += DeltaTime) >= TimeBetweenAudio)
		{
			AudioComponent->Play();
			Timer = 0.0f;
		}
	}
}

void ASpacedSoundCue::Enable()
{
	if (SoundToPlay != nullptr)
	{
		AudioComponent->SetSound(SoundToPlay);
		AudioComponent->Play();

		SetActorTickEnabled(true);
	}
	else
	{
		SetActorTickEnabled(false);
	}
}

void ASpacedSoundCue::Disable()
{
	AudioComponent->Stop();
	SetActorTickEnabled(false);
}
