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
	
	/* 
	 * Search for and play the song with the requested name.
	 *
	 * Returns true if the song was found, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = Audio)
	void PlaySong(USoundCue* Song);

	UFUNCTION(BlueprintCallable, Category = Audio)
	void PlayAudioCue(USoundCue* Voiceline);

	UFUNCTION(BlueprintCallable, Category = Audio)
	void PlayThemeSong();

	UFUNCTION()
	virtual void Initialise() override;

private:

	UPROPERTY(EditDefaultsOnly, Category = Settings)
	float MusicVolume = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = Settings)
	float AudioVolume = 1.0;

	UPROPERTY(EditDefaultsOnly, Category = Audio)
	USoundCue* ThemeSong;

	/* Audio component required to play songs. */
	UPROPERTY(EditDefaultsOnly, Category = Audio)
	UAudioComponent* SoundtrackPlayer;

	/* Audio component to play audio cues e.g. tutorial, hints */
	UPROPERTY(EditDefaultsOnly, Category = Audio)
	UAudioComponent* AudioCuePlayer;

	/* List of game songs to play. */
	TArray<USoundCue*> Soundtrack;

	/* List of audio cues to play. */
	TArray<USoundCue*> AudioCues;

	USoundCue* CurrentSong;
};
