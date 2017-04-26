// Fill out your copyright notice in the Description page of Project Settings.

#include "Eros.h"
#include "LevelChangeTrigger.h"
#include "Character/ErosCharacter.h"
#include "Engine/LevelStreaming.h"
#include "ErosGameMode.h"


// Sets default values
ULevelChangeTrigger::ULevelChangeTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = false;
	
	LevelsUpdated = 0;

	LevelsVisibleOnLoad = true;
	LevelsBlockOnLoad = false;
}

// Called when the game starts or when spawned
void ULevelChangeTrigger::BeginPlay()
{
	Super::BeginPlay();

	UBoxComponent* CollisionBox = Cast<UBoxComponent>(GetOwner()->GetComponentByClass(UBoxComponent::StaticClass()));
	if (CollisionBox)
	{
		FScriptDelegate OnOverlapDelegate;
		OnOverlapDelegate.BindUFunction(this, FName("OnBeginOverlap"));
		CollisionBox->OnComponentBeginOverlap.Add(OnOverlapDelegate);
	}
}

void ULevelChangeTrigger::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	AErosCharacter* Player = Cast<AErosCharacter>(OtherActor);

	if (!Player) return;

	if (Player->GetCapsuleComponent() == OtherComp)
	{
		LevelsUpdated = 0;
		UpdateNextlevel();
	}
}


void ULevelChangeTrigger::UpdateNextlevel()
{
	// Set latent info to call this function when the level load/unload operation completes
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = "UpdateNextlevel";
	LatentInfo.UUID = 1;
	LatentInfo.Linkage = 0;

	if (LevelsUpdated < LevelsToLoad.Num())
	{
		ULevelStreaming* SelectedLevel = UGameplayStatics::GetStreamingLevel(GetWorld(), LevelsToLoad[LevelsUpdated]);
		// If level should be visible after load, only bother loading if it is NOT visible. If level should be invisible after load only load if it is not loaded
		if (SelectedLevel && ((!SelectedLevel->IsLevelLoaded() && !LevelsVisibleOnLoad) || (!SelectedLevel->IsLevelVisible() && LevelsVisibleOnLoad)))
		{
			UE_LOG(LogTemp, Warning, TEXT("Loading Level"));
			UGameplayStatics::LoadStreamLevel(this, LevelsToLoad[LevelsUpdated], LevelsVisibleOnLoad, LevelsBlockOnLoad, LatentInfo);
		}
	}
	else if ((LevelsUpdated - LevelsToLoad.Num()) < LevelsToUnload.Num())
	{
		ULevelStreaming* SelectedLevel = UGameplayStatics::GetStreamingLevel(GetWorld(), LevelsToUnload[LevelsUpdated - LevelsToLoad.Num()]);
		if(SelectedLevel && SelectedLevel->IsLevelLoaded())
		{
			UE_LOG(LogTemp, Warning, TEXT("Unloading Level"));
			UGameplayStatics::UnloadStreamLevel(this, LevelsToUnload[LevelsUpdated - LevelsToLoad.Num()], LatentInfo);
		}
	}
	else
	{
		// Activate this level load as a checkpoint
		AErosCharacter* PlayerCharacter = Cast<AErosCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		if (PlayerCharacter)
		{
			Cast<AErosGameMode>(GetWorld()->GetAuthGameMode())->UpdateRespawnInformation(GetOwner(), PlayerCharacter);
		}
	}

	++LevelsUpdated;
}
