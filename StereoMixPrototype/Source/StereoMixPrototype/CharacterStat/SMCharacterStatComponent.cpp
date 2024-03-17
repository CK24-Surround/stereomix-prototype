// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterStat/SMCharacterStatComponent.h"

#include "Log/SMLog.h"
#include "Net/UnrealNetwork.h"

USMCharacterStatComponent::USMCharacterStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void USMCharacterStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USMCharacterStatComponent, BaseStat);
	DOREPLIFETIME(USMCharacterStatComponent, CurrentPostureGauge);
}

void USMCharacterStatComponent::AddCurrentPostureGauge(float InCurrentPostureGauge)
{
	const float NewPostureGauge = FMath::Clamp(CurrentPostureGauge + InCurrentPostureGauge, 0.0f, BaseStat.MaxPostureGauge);
	SetCurrentPostureGauge(NewPostureGauge);
}

void USMCharacterStatComponent::ClearPostureGauge()
{
	SetCurrentPostureGauge(0.0f);
}

void USMCharacterStatComponent::OnRep_BaseStat()
{
	NET_COMP_LOG(LogSMStat, Log, TEXT("베이스 스탯 변경 감지"));
}

void USMCharacterStatComponent::OnRep_CurrentPostureGauge()
{
	NET_COMP_LOG(LogSMStat, Log, TEXT("체간 게이지 변경 감지"));
	OnChangedPostureGauge.Broadcast(CurrentPostureGauge, BaseStat.MaxPostureGauge);

	if (CurrentPostureGauge >= BaseStat.MaxPostureGauge)
	{
		NET_COMP_LOG(LogSMStat, Log, TEXT("체간 게이지가 가득참"));
		OnZeroPostureGauge.Broadcast();
	}
}
