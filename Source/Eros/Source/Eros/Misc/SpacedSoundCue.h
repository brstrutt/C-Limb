#pragma once

#include "GameFramework/Actor.h"
#include "SpacedSoundCue.generated.h"

class USoundCue;

UCLASS()
class EROS_API ASpacedSoundCue : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpacedSoundCue();
	
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Spaced Audio")
	virtual void Enable();

	UFUNCTION(BlueprintCallable, Category = "Spaced Audio")
	virtual void Disable();

private:

	UPROPERTY(EditAnywhere, Category = "Spaced Audio Settings")
	bool bStartEnabled = true;

	UPROPERTY(EditAnywhere, Category = "Spaced Audio Settings")
	float TimeBetweenAudio;

	UPROPERTY(EditAnywhere, Category = "Spaced Audio Settings")
	USoundCue* SoundToPlay;

	UPROPERTY(VisibleAnywhere, Category = "Spaced Audio Settings")
	UAudioComponent* AudioComponent;

	float Timer;
};
