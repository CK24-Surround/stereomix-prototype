// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/SMCharacterStat.h"
#include "SMCharacterStatComponent.generated.h"


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
	FORCEINLINE const FSMCharacterStat& GetStat() const { return Stat; }
	FORCEINLINE void SetStat(const FSMCharacterStat& InCharacterStat) { Stat = InCharacterStat; }

	FORCEINLINE float GetCurrentPostureGauge() const { return CurrentPostureGauge; }
	FORCEINLINE void AddCurrentPostureGauge(float InCurrentPostureGauge);
	FORCEINLINE void SetCurrentPostureGauge(float InCurrentPostureGauge) { CurrentPostureGauge = InCurrentPostureGauge; }

protected:
	UFUNCTION()
	void OnRep_Stat();
	
	UPROPERTY(Transient, EditAnywhere, ReplicatedUsing = OnRep_Stat, Category = "Stat")
	FSMCharacterStat Stat;

	UPROPERTY()
	float CurrentPostureGauge;
// ~End of Stat Section
};
