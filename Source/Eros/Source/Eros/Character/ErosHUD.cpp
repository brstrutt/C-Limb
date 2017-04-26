#include "Eros.h"
#include "ErosHUD.h"

void AErosHUD::BroadcastNewTextCue(TArray<FString> Text, float Time)
{
	OnNewTextCue.Broadcast(Text, Time);
}