#include "Eros.h"

#include "../Character/ErosHUD.h"
#include "AudioManager.h"
#include "Misc/GameCue.h"
#include "GameCuesManager.h"
#include "Sound/SoundCue.h"

AGameCuesManager::AGameCuesManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGameCuesManager::Initialise()
{
	Super::Initialise();

	Timer = 0.0;
	QueuedCues.Empty();
}

void AGameCuesManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Timer -= DeltaTime;
	
	if (Timer < 0.0f && !QueuedCues.IsEmpty())
	{
		UGameCue* Cue;
		if (QueuedCues.Dequeue(Cue) && IsValid(Cue))
		{
			PlayCue(*Cue);
		}
	}
}

void AGameCuesManager::PlayGameIntroCue()
{
	PlayCue(*TutorialGameCue.GetDefaultObject());
}

void AGameCuesManager::PlayCue(UGameCue& Cue)
{
	if (Timer > 0.0f)
	{
		// Cue already playing.
		UE_LOG(LogTemp, Warning, TEXT("Queuing Cue"));

		QueuedCues.Enqueue(&Cue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Playing Cue"));

		Timer = Cue.PlayTime;

		if (Cue.DisplayText.Num())
		{
			Cast<AErosHUD>(GetWorld()->GetFirstPlayerController()->GetHUD())->BroadcastNewTextCue(Cue.DisplayText, Cue.TextDisplayTime);
		}

		if (IsValid(Cue.SoundCue))
		{
			AErosManager::GetManager<AAudioManager>(GetWorld())->PlayAudioCue(Cue.SoundCue);
		}
	}
}