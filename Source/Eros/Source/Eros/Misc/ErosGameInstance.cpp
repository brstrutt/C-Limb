#include "Eros.h"
#include "ErosGameInstance.h"
#include "../Managers/ErosManager.h"

UErosGameInstance::UErosGameInstance()
{
	CurrentControlScheme = 0;
}

void UErosGameInstance::Initialise()
{
	UE_LOG(LogTemp, Warning, TEXT("Initialise managers"));

	ManagerInstances.Empty();

	for (int Index = 0; Index < ManagerTemplates.Num(); Index++)
	{
		AErosManager* Manager = Cast<AErosManager>(GetWorld()->SpawnActor(ManagerTemplates[Index].GetDefaultObject()->GetClass()));
		Manager->Initialise();

		UE_LOG(LogTemp, Warning, TEXT("New Manager!"));

		ManagerInstances.Add(Manager);
	}

	bInitialised = true;
}

void UErosGameInstance::Reset()
{
	UE_LOG(LogTemp, Warning, TEXT("Reseting managers!"));

	for (int Index = 0; Index < ManagerTemplates.Num(); Index++)
	{
		ManagerInstances[Index]->Initialise();
	}
}