// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "LevelChangeTrigger.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EROS_API ULevelChangeTrigger : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ULevelChangeTrigger();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/*
	* Called whenever the previous level finishes loading/unloading.
	*
	* Loads/unloads the next level.
	*/
	UFUNCTION(BlueprintCallable, Category = Game)
	void UpdateNextlevel();

private:
	/* If true then levels will be visible when they load. If false then they will not. */
	UPROPERTY(EditAnywhere, Category = Levels)
	bool LevelsVisibleOnLoad;

	/* If true then game will freeze whenever a level gets loaded. */
	UPROPERTY(EditAnywhere, Category = Levels)
	bool LevelsBlockOnLoad;

	/* The levels in this array will be loaded on collision. */
	UPROPERTY(EditAnywhere, Category = Levels)
	TArray<FName> LevelsToLoad;

	/* The levels in this array will be unloaded on collision. */
	UPROPERTY(EditAnywhere, Category = Levels)
	TArray<FName> LevelsToUnload;
	
	int LevelsUpdated;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
};
