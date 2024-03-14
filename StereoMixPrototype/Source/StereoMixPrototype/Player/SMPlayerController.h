// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SMPlayerController.generated.h"

class UInputAction;
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
	virtual void SetupInputComponent() override;

protected: // Exit Section
	UFUNCTION(BlueprintImplementableEvent)
	void Exit();
	
	UPROPERTY(EditAnywhere, Category = "Ref(Input)")
	TObjectPtr<UInputAction> ExitAction;
};
