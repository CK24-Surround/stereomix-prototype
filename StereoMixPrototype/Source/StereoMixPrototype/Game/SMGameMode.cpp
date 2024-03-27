// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SMGameMode.h"

#include "Data/AssetPath.h"
#include "Design/SMGameDesignData.h"
#include "Log/SMLog.h"

ASMGameMode::ASMGameMode()
{
	static ConstructorHelpers::FObjectFinder<USMGameDesignData> DesignDataFinder(GAME_DESIGN_DATA);
	if (DesignDataFinder.Object)
	{
		DesignData = DesignDataFinder.Object;
	}

	check(DesignData);
	
	RoundTime = DesignData->RoundTime;
}

void ASMGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ASMGameMode::StartMatch()
{
	Super::StartMatch();

	RoundTimerStart();
	
	NET_LOG(LogSMNetwork, Log, TEXT("게임 시작"));
}

void ASMGameMode::GameResult()
{
	RemainRoundTime = DesignData->ResultTime;
	EndMatch();

	OnTriggerResult.Broadcast();

	NET_LOG(LogSMNetwork, Log, TEXT("게임 종료 및 결과창 출력"));
}

void ASMGameMode::FinishGame()
{
	RoundTimeEnd();

	GetWorld()->ServerTravel(TEXT("/Game/StereoMixPrototype/Level/L_Main"));
}

void ASMGameMode::RoundTimerStart()
{
	CurrentInGameState = EInGameState::Playing;
	RemainRoundTime = RoundTime;
	
	GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &ASMGameMode::PerformRoundTime, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void ASMGameMode::PerformRoundTime()
{
	--RemainRoundTime;
	(void)OnChangeRemainRoundTime.ExecuteIfBound();
	
	if (RemainRoundTime <= 0)
	{
		if (GetMatchState() == MatchState::InProgress)
		{
			GameResult();
		}
		else if (GetMatchState() == MatchState::WaitingPostMatch)
		{
			FinishGame();
		}
	}
}

void ASMGameMode::RoundTimeEnd()
{
	GetWorldTimerManager().ClearTimer(RoundTimerHandle);
}
