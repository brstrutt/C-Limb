#include "Eros.h"

#include "../Character/ErosCharacter.h"
#include "../Character/ErosController.h"
#include "CinematicTrigger.h"
#include "Runtime/LevelSequence/Public/LevelSequence.h"
#include "Runtime/LevelSequence/Public/LevelSequencePlayer.h"

void UCinematicTrigger::OverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!bHasActivated && Cast<AErosCharacter>(OtherActor))
	{
		bHasActivated = true;

		if (bDisablePlayerInput)
		{
			// Disable character input while the cinematic players.
			Cast<AErosController>(GetWorld()->GetFirstPlayerController())->SetInputEnabled(false);
		}

		PlayNextSequence();
	}
}

void UCinematicTrigger::PlayNextSequence()
{
	if (SequenceIndex < Sequences.Num())
	{
		// Play level sequence showing light change / door open.
		FLevelSequencePlaybackSettings Settings;
		Player = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), Sequences[SequenceIndex], Settings);
		Player->Play();

		FScriptDelegate SequenceDelegate;
		SequenceDelegate.BindUFunction(this, "PlayNextSequence");

		Player->OnStop.Clear();
		Player->OnStop.Add(SequenceDelegate);

		SequenceIndex++;
	}
	else
	{
		// Disable character input while the cinematic players.
		Cast<AErosController>(GetWorld()->GetFirstPlayerController())->SetInputEnabled(true);
	}
}