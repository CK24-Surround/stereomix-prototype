// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMPostureGaugeWidget.generated.h"

/**
 * 
 */
UCLASS()
class STEREOMIXPROTOTYPE_API USMPostureGaugeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdatePostureGauge(float InCurrentPostureGauge, float MaxPostureGauge);
};
