// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/SMTeam.h"
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

	/** 캐릭터 기준으로 마우스 포인터가 가리키는 방향을 반환합니다. */
	FVector GetMousePointingDirection() const;

	/** 현재 마우스 포인터의 위치를 반환합니다 */
	FVector GetMouseCursorLocation() const;

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
	void ProcessResult(ESMTeam InVictoryTeam);

	UPROPERTY()
	TObjectPtr<USMBattleHUDWidget> BattleHUD;
	// ~End of Widget Section
};
