#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMBattleHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class STEREOMIXPROTOTYPE_API USMBattleHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateFutureBassTeamScore(int32 InFutureBassTeamScore);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateRockTeamScore(int32 InRockTeamScore);

	void UpdateRemainRoundTime(int32 InRemainRoundTime);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateRemainRoundMinutesAndSeconds(const FString& InMinutes, const FString& InSeconds);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowResultDraw();
	
	UFUNCTION(BlueprintImplementableEvent)
	void ShowResult(bool bIsVictory);
};
