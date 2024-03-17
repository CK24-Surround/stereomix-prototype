// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/SMCharacterStat.h"
#include "SMCharacterStatComponent.generated.h"

DECLARE_LOG_CATEGORY_CLASS(LogSMStat, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChangedPostureGaugeSignature, float, CurrentPostureGauge, float, MaxPostureGauge);
DECLARE_MULTICAST_DELEGATE(FOnZeroPostureGaugeSignature);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEREOMIXPROTOTYPE_API USMCharacterStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USMCharacterStatComponent();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

// ~Stat Section
public:
	FORCEINLINE const FSMCharacterStat& GetBaseStat() const { return BaseStat; }
	FORCEINLINE void SetBaseStat(const FSMCharacterStat& InCharacterStat) { BaseStat = InCharacterStat; }

	FORCEINLINE float GetCurrentPostureGauge() const { return CurrentPostureGauge; }
	void AddCurrentPostureGauge(float InCurrentPostureGauge);
	
	FORCEINLINE void SetCurrentPostureGauge(float InCurrentPostureGauge)
	{
		if (GetOwnerRole() == ROLE_Authority)
		{
			CurrentPostureGauge = InCurrentPostureGauge;

			OnRep_CurrentPostureGauge();
		}
	}

	void ClearPostureGauge();

protected:
	UFUNCTION()
	void OnRep_BaseStat();

	UPROPERTY(Transient, BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_BaseStat, Category = "Stat")
	FSMCharacterStat BaseStat;

	UFUNCTION()
	void OnRep_CurrentPostureGauge();

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentPostureGauge)
	float CurrentPostureGauge;
// ~End of Stat Section

// ~Delegate Section
public:
	UPROPERTY(BlueprintAssignable)
	FOnChangedPostureGaugeSignature OnChangedPostureGauge;

	FOnZeroPostureGaugeSignature OnZeroPostureGauge;
// ~End of Delegate Section
};
