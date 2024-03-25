// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SMGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class STEREOMIXPROTOTYPE_API USMGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	USMGameInstance();
	
public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FString GetPlayerName() const
	{
		return PlayerName;
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetPlayerName(const FString& InName)
	{
		PlayerName = InName;
	}

protected:
	FString PlayerName;
};
