#pragma once

#include "../Misc/ErosGameInstance.h"
#include "GameFramework/Actor.h"
#include "ErosManager.generated.h"

UCLASS()
class EROS_API AErosManager : public AActor
{
	GENERATED_BODY()	

public:

	/* Utility function to get a manager in the game. May be null! */
	template<typename T> static T* GetManager(UWorld* World)
	{
		return Cast<UErosGameInstance>(World->GetGameInstance())->GetManager<T>();
	}

	UFUNCTION(BlueprintCallable, Category = Manager)
	virtual void Initialise();
};
