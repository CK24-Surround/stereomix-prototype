// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SMPlayerController.generated.h"

class USMBattleHUDWidget;
class UInputAction;
class USMCharacterAssetData;
class USMPlayerControllerAssetData;

DECLARE_LOG_CATEGORY_CLASS(LogSMPlayerController, Log, All);

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

protected:
	UPROPERTY()
	TObjectPtr<const USMPlayerControllerAssetData> AssetData;

protected: // Exit Section
	UFUNCTION(BlueprintImplementableEvent)
	void Exit();
	
	UPROPERTY(EditAnywhere, Category = "Ref(Input)")
	TObjectPtr<UInputAction> ExitAction;

// ~Widget Section
protected:
	UPROPERTY()
	TObjectPtr<USMBattleHUDWidget> BattleHUD;
// ~End of Widget Section
};
