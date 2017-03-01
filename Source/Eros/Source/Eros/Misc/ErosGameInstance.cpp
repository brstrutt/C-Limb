#include "Eros.h"
#include "ErosGameInstance.h"
#include "../Managers/ErosManager.h"

void UErosGameInstance::Initialise()
{
	if (bInitialised) { return; }

	for (int Index = 0; Index < ManagerTemplates.Num(); Index++)
	{
		ManagerInstances.Add(Cast<AErosManager>(GetWorld()->SpawnActor(ManagerTemplates[Index].GetDefaultObject()->GetClass())));
	}
}