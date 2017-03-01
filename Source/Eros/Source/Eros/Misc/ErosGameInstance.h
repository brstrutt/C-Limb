#pragma once

#include "Engine/GameInstance.h"
#include "ErosGameInstance.generated.h"

class AErosManager;

/* 
* A game instance is a persistent class that can be access from the game start to game end.
*
* The game instance will be used as our accessor to mananager classes.
*/
UCLASS()
class EROS_API UErosGameInstance : public UGameInstance
{
	GENERATED_BODY()	

public:

	/* Initialise the game managers. */
	void Initialise();

	/* Returns a manager of the requested type. May be null! */
	template<typename T> T* GetManager()
	{
		if (!bInitialised)
		{
			Initialise();
		}

		for (int Index = 0; Index < ManagerInstances.Num(); Index++)
		{
			if (ManagerInstances[Index]->IsA(T::StaticClass()))
			{
				return Cast<T>(ManagerInstances[Index]);
			}
		}
		return nullptr;
	}

private:

	/* Only initialise the game instance managers once. */
	bool bInitialised;
	
	/* List of template managers to spawn. */
	UPROPERTY(EditDefaultsOnly, Category = Managers)
	TArray<TSubclassOf<AErosManager>> ManagerTemplates;
		
	/* List of spawned managers. */
	TArray<AErosManager*> ManagerInstances;
};