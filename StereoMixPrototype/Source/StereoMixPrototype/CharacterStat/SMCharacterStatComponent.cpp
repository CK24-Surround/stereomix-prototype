// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterStat/SMCharacterStatComponent.h"

#include "Net/UnrealNetwork.h"

USMCharacterStatComponent::USMCharacterStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void USMCharacterStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USMCharacterStatComponent, Stat);
}

void USMCharacterStatComponent::AddCurrentPostureGauge(float InCurrentPostureGauge)
{
	CurrentPostureGauge = FMath::Clamp(CurrentPostureGauge + InCurrentPostureGauge, 0.0f, Stat.MaxPostureGauge);
}

void USMCharacterStatComponent::OnRep_Stat() {}
