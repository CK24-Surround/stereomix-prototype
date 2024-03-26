// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SMGameMode.generated.h"

DECLARE_DELEGATE(FOnChangeRemainRoundTimeSignature);

UENUM(BlueprintType)
enum class EInGameState
{
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

protected:
	virtual void BeginPlay() override;
	
protected:
	void RoundTimerStart();

	void PerformRoundTime();

public:
	FORCEINLINE int32 GetRemainRoundTime() { return RemainRoundTime; }

protected:
	EInGameState CurrentInGameState;

	FTimerHandle RoundTimerHandle;
	int32 RoundTime = 300;
	int32 RemainRoundTime = 0;

// ~Delegate Section
public:
	FOnChangeRemainRoundTimeSignature OnChangeRemainRoundTime;
// ~End of Delegate Section
};
