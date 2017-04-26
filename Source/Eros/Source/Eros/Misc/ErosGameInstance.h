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

	UErosGameInstance();

	/* Initialise the game managers. */
	UFUNCTION(BlueprintCallable, Category = "ManagerControl")
	void Initialise();

	UFUNCTION(BlueprintCallable, Category = "ManagerControl")
	void Reset();
	
	/* Returns a manager of the requested type. May be null! */
	template<typename T> T* GetManager()
	{
		if (!bInitialised)
		{
			Initialise();
		}

		for (int Index = 0; Index < ManagerInstances.Num(); Index++)
		{
			if (IsValid(ManagerInstances[Index]))
			{
				if (ManagerInstances[Index]->IsA(T::StaticClass()))
				{
					return Cast<T>(ManagerInstances[Index]);
				}
			}			
		}
		return nullptr;
	}

	/* Hack to allow keyboard/controller controls to be specified. Only affects widgets. 0 = controller. 1 = keyboard*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Controls)
	int CurrentControlScheme;

private:

	/* Only initialise the game instance managers once. */
	bool bInitialised = false;
	
	/* List of template managers to spawn. */
	UPROPERTY(EditDefaultsOnly, Category = Managers)
	TArray<TSubclassOf<AErosManager>> ManagerTemplates;
		
	/* List of spawned managers. */
	TArray<AErosManager*> ManagerInstances;
};