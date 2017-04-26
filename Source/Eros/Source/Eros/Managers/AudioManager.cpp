#include "Eros.h"
#include "AudioManager.h"
#include "Sound/SoundCue.h"

AAudioManager::AAudioManager()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = SoundtrackPlayer = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));

	AudioCuePlayer = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioCues"));
	AudioCuePlayer->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
}

void AAudioManager::Initialise()
{
	Super::Initialise();

	UE_LOG(LogTemp, Warning, TEXT("FUCKEN STOP THE AUDIO"));

	AudioCuePlayer->Stop();
	PlayThemeSong();
}

void AAudioManager::PlaySong(USoundCue* Song)
{
	// Already playing.
	if (!IsValid(Song) || Song == CurrentSong) { return; }

	//// Find the Named song in the Soundtrack array
	//for (int Index = 0; Index < Soundtrack.Num(); Index++)
	//{
	//	if (Soundtrack[Index]->GetName() == Song->GetName())
	//	{
	//		// Play the song
	//		SoundtrackPlayer->SetSound(Soundtrack[Index]);
	//		
	//		if (!SoundtrackPlayer->IsPlaying())
	//		{
	//			SoundtrackPlayer->SetVolumeMultiplier(MusicVolume);
	//			SoundtrackPlayer->Play();
	//		}

	//		CurrentSong = Song;

	//		return;
	//	}
	//}

	//// Didn't exist, add it.
	//Soundtrack.Add(Song);

	SoundtrackPlayer->SetSound(Song);
	SoundtrackPlayer->SetVolumeMultiplier(MusicVolume);
	SoundtrackPlayer->Play();
}

void AAudioManager::PlayAudioCue(USoundCue* Voiceline)
{
	if (!IsValid(Voiceline)) { return; }

	//// Find the Named song in the Soundtrack array
	//for (int Index = 0; Index < AudioCues.Num(); Index++)
	//{
	//	if (AudioCues[Index]->GetName() == Voiceline->GetName())
	//	{
	//		// Play the song
	//		AudioCuePlayer->SetSound(AudioCues[Index]);

	//		if (!AudioCuePlayer->IsPlaying())
	//		{
	//			AudioCuePlayer->SetVolumeMultiplier(AudioVolume);
	//			AudioCuePlayer->Play();
	//		}

	//		return;
	//	}
	//}

	//// Didn't exist, add it.
	//AudioCues.Add(Voiceline);

	AudioCuePlayer->SetSound(Voiceline);
	AudioCuePlayer->SetVolumeMultiplier(AudioVolume);
	AudioCuePlayer->Play();
}

void AAudioManager::PlayThemeSong()
{
	PlaySong(ThemeSong);
	SoundtrackPlayer->SetVolumeMultiplier(0.15f);
}