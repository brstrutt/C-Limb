// Fill out your copyright notice in the Description page of Project Settings.

#include "Eros.h"
#include "DoorSwitch.h"

ADoorSwitch::ADoorSwitch()
{
	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	Door->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
	DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrameMesh"));
	DoorFrame->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
	
	CloseDoorTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("CloseTriggerBox"));
	CloseDoorTrigger->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	OpenCloseAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("OpenCloseAudioComponent"));
	OpenCloseAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	bIsOpen = false;
}

void ADoorSwitch::CloseDoor()
{
	if (bIsOpen)
	{
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(Door, FVector(0, 0, 0), FRotator(FQuat(0, 0, 0, 1)), true, true, 0.5f, true, EMoveComponentAction::Move, LatentInfo);
		if (OpenCloseSound != nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("door audio... doordio"));
			OpenCloseAudioComponent->SetSound(OpenCloseSound);
			OpenCloseAudioComponent->Play();
		}
		bIsOpen = false;
	}
}

void ADoorSwitch::OpenDoor()
{
	UE_LOG(LogTemp, Log, TEXT("opening door"));
	if (!bIsOpen)
	{
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(Door, FVector(0, 0, 400), FRotator(FQuat(0, 0, 0, 1)), true, true, 0.5f, true, EMoveComponentAction::Move, LatentInfo);
		if (OpenCloseSound != nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("door audio... doordio"));
			OpenCloseAudioComponent->SetSound(OpenCloseSound);
			OpenCloseAudioComponent->Play();
		}
		bIsOpen = true;
	}
}

void ADoorSwitch::Activate()
{
	OpenDoor();
}