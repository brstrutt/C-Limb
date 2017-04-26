#include "Eros.h"
#include "ErosGameMode.h"
#include "EngineUtils.h"
#include "Character/ErosCharacter.h"
#include "AI/DroneAICharacter.h"

AErosGameMode::AErosGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/ErosCharacterBP"));

	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	LastCheckpoint = {
		FVector(0.0f, 0.0f, 0.0f),
		FRotator(0.0f, 0.0f, 0.0f) 
	};

	ArmClass = nullptr;
	LegClass = nullptr;

	//DroneRespawnStates = TArray<FDroneRespawnState>();
}

void AErosGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Attach OnPlayerDestroyed as a delegate to the main characters destroyed event
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	if (PlayerCharacter)
	{
		PlayerCharacter->OnDestroyed.AddDynamic(this, &AErosGameMode::OnPlayerDestroyed);
		AErosCharacter* TempCharacter = Cast<AErosCharacter>(PlayerCharacter);
		if (TempCharacter)
		{
			TempCharacter->OnProstheticSwapped.AddDynamic(this, &AErosGameMode::OnProstheticSwapped);
		}
		UpdateRespawnInformation(PlayerCharacter, TempCharacter);
	}	
}

void AErosGameMode::UpdateRespawnInformation(AActor* CheckpointHit, AErosCharacter* PlayerCharacter)
{
	SetCheckpointData(CheckpointHit->GetActorLocation(), CheckpointHit->GetActorRotation());
	SetRespawnProsthetics(PlayerCharacter);
	//SetDroneStates();
}

void AErosGameMode::SetRespawnProsthetics(AErosCharacter* PlayerCharacter)
{
	AProsthetic* CurrentArm = PlayerCharacter->GetArmSocket()->GetProsthetic();
	AProsthetic* CurrentLeg = PlayerCharacter->GetLegSocket()->GetProsthetic();

	ArmClass = nullptr;
	LegClass = nullptr;

	if (CurrentArm)
	{
		ArmClass = CurrentArm->GetClass();
	}
	if (CurrentLeg)
	{
		LegClass = CurrentLeg->GetClass();
	}
}

void AErosGameMode::SetCheckpointData(FVector LastCheckpointLocation, FRotator LastCheckpointRotation)
{	
	LastCheckpoint = {
		LastCheckpointLocation,
		LastCheckpointRotation 
	};
}

void AErosGameMode::SetDroneStates()
{
	///DroneRespawnStates.Empty();
	///loop through all drones extracting the information
	///for (TActorIterator<ADroneAICharacter> DroneIter(GetWorld()); DroneIter; ++DroneIter)
	///{
	///	DroneRespawnStates.Add(DroneIter->GetCurrentDroneState());
	///}
}

void AErosGameMode::OnProstheticSwapped(AErosCharacter* PlayerCharacter, AProsthetic* NewProsthetic)
{
	UE_LOG(LogTemp, Warning, TEXT("PROSTHETIC INFO UPDATING!"));
	SetRespawnProsthetics(PlayerCharacter);
}

void AErosGameMode::OnPlayerDestroyed(AActor* DestroyedActor)
{
	ResetDrones();
	RespawnPlayer(ArmClass, LegClass);
}

void AErosGameMode::RespawnPlayer(UClass* ArmProsthetic, UClass* LegProsthetic)
{
	if (DefaultPawnClass == nullptr) { return; }

	//Disable input (if input is receive when there is no character Unreal crashes)
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	DisableInput(PlayerController);

	AErosCharacter* NewCharacter = Cast<AErosCharacter>(GetWorld()->SpawnActor(DefaultPawnClass, &LastCheckpoint.Location, &LastCheckpoint.Rotation));

	// Destroy whatever prosthetics spawn by default (some way to spawn the class specifying the starting prosthetics would be nice).
	if (AProsthetic* TempProsthetic = NewCharacter->GetArmSocket()->DetachProsthetic())
	{
		TempProsthetic->Destroy();
	}

	if (AProsthetic* TempProsthetic = NewCharacter->GetLegSocket()->DetachProsthetic())
	{
		TempProsthetic->Destroy();
	}

	// Attach the chosen prosthetics
	if (ArmProsthetic)
	{
		NewCharacter->AttachProsthetic(*GetWorld()->SpawnActor<AProsthetic>(ArmProsthetic));
	}

	if (LegProsthetic)
	{
		NewCharacter->AttachProsthetic(*GetWorld()->SpawnActor<AProsthetic>(LegProsthetic));
	}
	else
	{
		NewCharacter->ChangeState(ECharacterState::CS_Crawling);
	}
	
	// Rebind the delegates
	NewCharacter->OnDestroyed.AddDynamic(this, &AErosGameMode::OnPlayerDestroyed);
	NewCharacter->OnProstheticSwapped.AddDynamic(this, &AErosGameMode::OnProstheticSwapped);

	//Re-possess character and enable input
	PlayerController->Possess(NewCharacter);
	EnableInput(PlayerController);
}

void AErosGameMode::ResetDrones()
{
	//Loop through drones resetting their info
	for (TActorIterator<ADroneAICharacter> DroneIter(GetWorld()); DroneIter; ++DroneIter)
	{
		DroneIter->ResetDroneState();
	}

	///for (int i = 0; i < DroneRespawnStates.Num(); ++i)
	///{
	///	DroneRespawnStates[i].TheDrone->SetCurrentDroneState(DroneRespawnStates[i]);
	///}
}