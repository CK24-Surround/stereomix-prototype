// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SMPlayerController.generated.h"

class USMCharacterAssetData;
/**
 * 
 */
UCLASS()
class STEREOMIXPROTOTYPE_API ASMPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASMPlayerController();
	
protected:
	virtual void BeginPlay() override;
};
