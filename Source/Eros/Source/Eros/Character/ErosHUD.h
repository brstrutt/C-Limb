#pragma once

#include "GameFramework/HUD.h"
#include "ErosHUD.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNewTextCue, TArray<FString> const&, Text, float, Time);

/* 
	* The HUD is used to store data between the UI and the player.
	*
	* It will also serve as the 'menu manger' or 'wdiget manager' so that we can handle UI display logic in one central location. 
*/
UCLASS()
class EROS_API AErosHUD : public AHUD
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FNewTextCue OnNewTextCue;

	UFUNCTION()
	void BroadcastNewTextCue(TArray<FString> Text, float Time);
};
