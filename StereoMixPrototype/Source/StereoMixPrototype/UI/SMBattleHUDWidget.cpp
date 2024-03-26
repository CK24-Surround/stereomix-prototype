#include "SMBattleHUDWidget.h"

void USMBattleHUDWidget::UpdateRemainRoundTime(int32 InRemainRoundTime)
{
	FString Minutes = FString::FromInt(InRemainRoundTime / 60);
	FString Seconds = FString::FromInt(InRemainRoundTime % 60);

	if (Minutes.Len() < 2)
	{
		Minutes = "0" + Minutes;
	}

	if (Seconds.Len() < 2)
	{
		Seconds = "0" + Seconds;
	}

	UpdateRemainRoundMinutesAndSeconds(Minutes, Seconds);
}