#pragma once

#include "GameFramework/GameStateBase.h"
#include "ErosGameState.generated.h"

UCLASS()
class EROS_API AErosGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;
		
};
