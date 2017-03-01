#include "Eros.h"
#include "AudioManager.h"
#include "Sound/SoundCue.h"

AAudioManager::AAudioManager()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
}

void AAudioManager::BeginPlay()
{
	Super::BeginPlay();

	PlaySong("FrontLineAssemlby_Vanished");
}

bool AAudioManager::PlaySong(const FString& Name)
{
	// Find the Named song in the Soundtrack array
	for (int Index = 0; Index < Soundtrack.Num(); Index++)
	{
		if (Soundtrack[Index]->GetName().Compare(Name))
		{
			// Play the song
			AudioComponent->SetSound(Soundtrack[Index]);
			
			if (!AudioComponent->IsPlaying())
			{
				AudioComponent->Play();
			}

			return true;
		}
	}

	return false;
}