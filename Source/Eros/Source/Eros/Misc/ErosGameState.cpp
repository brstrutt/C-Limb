#include "Eros.h"
#include "ErosGameInstance.h"
#include "ErosGameState.h"

void AErosGameState::BeginPlay()
{
	Super::BeginPlay();

	// Initialise game managers.
	//Cast<UErosGameInstance>(GetWorld()->GetGameInstance())->Initialise();
}