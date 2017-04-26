#pragma once

#include "Managers/ErosManager.h"
#include "GameCuesManager.generated.h"

class UGameCue;

UCLASS()
class EROS_API AGameCuesManager : public AErosManager
{
	GENERATED_BODY()

public:

	AGameCuesManager();

	/* Quick and nasty way to play a game cue in the reception. */
	UFUNCTION(BlueprintCallable, Category = "Game Cues")
	void PlayGameIntroCue();

	UFUNCTION(BlueprintCallable, Category = Manager)
	virtual void Initialise() override;

	virtual void Tick(float DeltaTime) override;

	void PlayCue(UGameCue& Cue);

private:

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameCue> TutorialGameCue;

	float Timer;

	TQueue<UGameCue*> QueuedCues;
};
