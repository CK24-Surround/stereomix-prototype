// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SMGameMode.h"

void ASMGameMode::BeginPlay()
{
	Super::BeginPlay();

	RoundTimerStart();
}

void ASMGameMode::RoundTimerStart()
{
	RemainRoundTime = RoundTime;
	
	GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &ASMGameMode::PerformRoundTime, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void ASMGameMode::PerformRoundTime()
{
	--RemainRoundTime;

	(void)OnChangeRemainRoundTime.ExecuteIfBound();
}
