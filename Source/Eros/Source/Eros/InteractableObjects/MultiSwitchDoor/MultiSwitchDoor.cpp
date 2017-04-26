#include "Eros.h"

#include "../../Character/ErosController.h"
#include "../../Managers/GameCuesManager.h"
#include "MultiSwitch.h"
#include "MultiSwitchDoor.h"
#include "Runtime/LevelSequence/Public/LevelSequence.h"
#include "Runtime/LevelSequence/Public/LevelSequencePlayer.h"

AMultiSwitchDoor::AMultiSwitchDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	TemporaryDoor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	TemporaryDoor->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	OpenCloseAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("OpenCloseAudioComponent"));
	OpenCloseAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
}

void AMultiSwitchDoor::BeginPlay()
{
	Super::BeginPlay();

	// Set all switches to be inactive
	for (int Index = 0; Index < Switches.Num(); ++Index)
	{
		Switches[Index].Switch->Initialise(*this);

		Switches[Index].ActiveActor->SetActorHiddenInGame(true);
		Switches[Index].InactiveActor->SetActorHiddenInGame(false);

		Switches[Index].bActive = false;

		// Disable all but the first switch.
		if (Index > 0)
		{
			Switches[Index].Switch->SetInteractable(false);
		}
	}

	bUnlocked = false;
}

bool AMultiSwitchDoor::TryUnlock()
{
	if (bUnlocked) { return true; }

	for (int Index = 0; Index < Switches.Num(); ++Index)
	{
		if (!Switches[Index].bActive)
		{
			return false;
		}
	}

	//TemporaryDoor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//TemporaryDoor->SetVisibility(false);
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(TemporaryDoor, FVector(0, 0, 400), FRotator(FQuat(0, 0, 0, 1)), true, true, 0.5f, true, EMoveComponentAction::Move, LatentInfo);
	if (OpenCloseSound != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("door audio... doordio"));
		OpenCloseAudioComponent->SetSound(OpenCloseSound);
		OpenCloseAudioComponent->Play();
	}

	bUnlocked = true;

	return true;
}

void AMultiSwitchDoor::SwitchActivated(AMultiSwitch& Switch)
{
	for (int Index = 0; Index < Switches.Num(); ++Index)
	{
		if (Switches[Index].Switch == &Switch)
		{
			ToActivate = &Switches[Index];

			FTimerDynamicDelegate DelayedDelegate;
			DelayedDelegate.BindUFunction(this, FName("DelayedActivation"));
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, DelayedDelegate, 1.25f, false);

			// Play level sequence showing light change / door open.
			FLevelSequencePlaybackSettings Settings;
			Player = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), Sequence, Settings);
			Player->Play();

			FScriptDelegate SequenceDelegate;
			SequenceDelegate.BindUFunction(this, "SequenceComplete");
			Player->OnStop.Add(SequenceDelegate);

			// The game cue to play when the sequence ends.
			GameCueToPlay = Switches[Index].SequenceCompleteCue.GetDefaultObject();

			// Disable character input while the cinematic players.
			Cast<AErosController>(GetWorld()->GetFirstPlayerController())->SetInputEnabled(false);

			// Enable the next switch.
			if ((Index + 1) < Switches.Num())
			{
				Switches[Index + 1].Switch->SetInteractable(true);
			}

			break;
		}
	}
}

void AMultiSwitchDoor::DelayedActivation()
{
	if (ToActivate == nullptr) { return; }

	ToActivate->ActiveActor->SetActorHiddenInGame(false);
	ToActivate->InactiveActor->SetActorHiddenInGame(true);
	ToActivate->bActive = true;

	ToActivate = nullptr;

	// Attempt to unlock the door.
	TryUnlock();
}

void AMultiSwitchDoor::SequenceComplete()
{
	if (GameCueToPlay != nullptr)
	{
		AErosManager::GetManager<AGameCuesManager>(GetWorld())->PlayCue(*GameCueToPlay);
		Cast<AErosController>(GetWorld()->GetFirstPlayerController())->SetInputEnabled(true);

		GameCueToPlay = nullptr;
	}
}