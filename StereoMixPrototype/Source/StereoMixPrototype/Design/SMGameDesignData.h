// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SMGameDesignData.generated.h"

/**
 * 
 */
UCLASS()
class STEREOMIXPROTOTYPE_API USMGameDesignData : public UDataAsset
{
	GENERATED_BODY()

// ~Game Section
public:
	UPROPERTY(EditDefaultsOnly, Category = "Game", DisplayName = "라운드 당 시간(초)")
	int32 RoundTime = 300;

	UPROPERTY(EditDefaultsOnly, Category = "Game", DisplayName = "라운드 당 시간(초)")
	int32 ResultTime = 15;
// ~end of Game Section
};
