#pragma once

#include "UObject/NoExportTypes.h"
#include "GameCue.generated.h"

class USoundCue;

UCLASS(Blueprintable)
class EROS_API UGameCue : public UObject
{
	GENERATED_BODY()

public:
	 
	/* Total time to play the cue. Used to queue other cues. */
	UPROPERTY(EditAnywhere)
	float PlayTime;

	/* Text to display in the UI. Each array is a paragraph that is show in full in the UI. Each paragraph will be displayed for TextDisplayTime amount of time. */
	UPROPERTY(EditAnywhere)
	TArray<FString> DisplayText;

	/* How long to display the text in the UI for. */
	UPROPERTY(EditAnywhere)
	float TextDisplayTime;

	/* Name of the sound cue to play. */
	UPROPERTY(EditAnywhere)
	USoundCue* SoundCue;
};
