#pragma once

#include "ErosManager.h"
#include "AudioManager.generated.h"

class USoundCue;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EROS_API AAudioManager : public AErosManager
{
	GENERATED_BODY()

public:	

	AAudioManager();

	void BeginPlay() override;

	/* 
	 * Search for and play the song with the requested name.
	 *
	 * Returns true if the song was found, false otherwise.
	 */
	UFUNCTION()
	bool PlaySong(const FString& Name);

private:

	/* List of game songs to play. */
	UPROPERTY(EditDefaultsOnly, Category = Soundtrack)
	TArray<USoundCue*> Soundtrack;

	/* Audio component required to play songs. */
	UPROPERTY(EditDefaultsOnly, Category = Audio)
	UAudioComponent* AudioComponent;
};
