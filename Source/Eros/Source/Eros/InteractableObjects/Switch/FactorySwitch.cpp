// Fill out your copyright notice in the Description page of Project Settings.

#include "Eros.h"
#include "Character/ErosController.h"
#include "Engine/Light.h"
#include "Runtime/Engine/Classes/Animation/SkeletalMeshActor.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "Runtime/LevelSequence/Public/LevelSequence.h"
#include "Runtime/LevelSequence/Public/LevelSequencePlayer.h"
#include "FactorySwitch.h"


AFactorySwitch::AFactorySwitch()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFactorySwitch::BeginPlay()
{
	Super::BeginPlay();

	SetLightsActive(false);
	SetAnimationsActive(false);
	SetInteractable(true);
}

void AFactorySwitch::Activate()
{
	SetLightsActive(true);
	SetAnimationsActive(true);
	SetConveyorBeltActive(true);

	FLevelSequencePlaybackSettings Settings;
	Player = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), Sequence, Settings);
	Player->Play();

	FScriptDelegate SequenceDelegate;
	SequenceDelegate.BindUFunction(this, "SequenceDone");

	Player->OnStop.Clear();
	Player->OnStop.Add(SequenceDelegate);

	Cast<AErosController>(GetWorld()->GetFirstPlayerController())->SetInputEnabled(false);
}

void AFactorySwitch::SequenceDone()
{
	Cast<AErosController>(GetWorld()->GetFirstPlayerController())->SetInputEnabled(true);
}

void AFactorySwitch::SetLightsActive(bool Activate)
{
	for (int i = 0; i < ControlledLights.Num(); ++i)
	{
		ControlledLights[i]->GetLightComponent()->SetVisibility(Activate);
	}
}

void AFactorySwitch::SetAnimationsActive(bool Activate)
{	
	for (int i = 0; i < ControlledArms.Num(); ++i)
	{
		ControlledArms[i]->GetSkeletalMeshComponent()->GlobalAnimRateScale = Activate ? 0.5f : 0.0f;
	}
}

void AFactorySwitch::SetConveyorBeltActive(bool Activate)
{
	for (int i = 0; i < ConveyorBeltControllers.Num(); ++i)
	{
		ConveyorBeltControllers[i]->ActivateConveyor(Activate);
	}
}


