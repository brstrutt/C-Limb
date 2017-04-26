// Fill out your copyright notice in the Description page of Project Settings.

#include "Eros.h"
#include "TimerSwitchDoor.h"


// Sets default values
ATimerSwitchDoor::ATimerSwitchDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	DoorMesh->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	Sparks = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Sparks"));
	Sparks->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	OpenCloseAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("OpenCloseAudioComponent"));
	OpenCloseAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
}

// Called when the game starts or when spawned
void ATimerSwitchDoor::BeginPlay()
{
	Super::BeginPlay();

	bIsOpen = false;
}

void ATimerSwitchDoor::OpenDoor(float TimeOpenFor, ASwitch* NewControllingSwitch)
{
	UE_LOG(LogTemp, Log, TEXT("opening door"));

	ControllingSwitch = NewControllingSwitch;

	ControllingSwitch->SetInteractable(false);

	if (!bIsOpen)
	{
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(DoorMesh, FVector(0, 0, OpenHeight), FRotator(FQuat(0, 0, 0, 1)), true, true, 0.5f, true, EMoveComponentAction::Move, LatentInfo);
		if (OpenCloseSound != nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("door audio... doordio"));
			OpenCloseAudioComponent->SetSound(OpenCloseSound);
			OpenCloseAudioComponent->Play();
		}
		bIsOpen = true;

		if (TimeOpenFor > 0)
		{
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &ATimerSwitchDoor::CloseDoor, TimeOpenFor);
		}
		
		if (Sparks->Template)
		{
			Sparks->Activate();
		}
	}
}

void ATimerSwitchDoor::CloseDoor()
{
	if (bIsOpen)
	{
		ControllingSwitch->SetInteractable(true);
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(DoorMesh, FVector(0, 0, 0), FRotator(FQuat(0, 0, 0, 1)), true, true, 0.5f, true, EMoveComponentAction::Move, LatentInfo);
		if (OpenCloseSound != nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("door audio... doordio"));
			OpenCloseAudioComponent->SetSound(OpenCloseSound);
			OpenCloseAudioComponent->Play();
		}
		bIsOpen = false;
	}
}