// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SMGameMode.generated.h"

class USMGameDesignData;
DECLARE_DELEGATE(FOnChangeRemainRoundTimeSignature);

UENUM(BlueprintType)
enum class EInGameState
{
	PrePlaying,
	Playing,
	End
};

/**
 * 
 */
UCLASS()
class STEREOMIXPROTOTYPE_API ASMGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ASMGameMode();

protected:
	virtual void BeginPlay() override;

public:
	virtual void StartMatch() override;
	
	void GameResult();

	void FinishGame();

protected:
	UPROPERTY()
	TObjectPtr<USMGameDesignData> DesignData;
	
// ~Round Timer Section
protected:
	void RoundTimerStart();

	void PerformRoundTime();

	void RoundTimeEnd();

public:
	FORCEINLINE int32 GetRemainRoundTime() { return RemainRoundTime; }

protected:
	EInGameState CurrentInGameState = EInGameState::PrePlaying;

	FTimerHandle RoundTimerHandle;
	int32 RoundTime = 0.0f;
	int32 RemainRoundTime = 0.0f;
// ~End of Round Timer Section

// ~Delegate Section
public:
	FOnChangeRemainRoundTimeSignature OnChangeRemainRoundTime;
// ~End of Delegate Section
};
