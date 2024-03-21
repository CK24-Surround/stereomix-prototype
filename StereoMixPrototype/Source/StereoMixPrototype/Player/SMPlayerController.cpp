// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SMPlayerController.h"

#include "EnhancedInputComponent.h"
#include "SMPlayerControllerAssetData.h"
#include "Blueprint/UserWidget.h"
#include "Data/AssetPath.h"
#include "Game/SMGameState.h"
#include "Log/SMLog.h"
#include "UI/SMBattleHUDWidget.h"

ASMPlayerController::ASMPlayerController()
{
	static ConstructorHelpers::FObjectFinder<USMPlayerControllerAssetData> DA_PlayerControllerAsset(PLAYER_CONTROLLER_ASSET_PATH);
	if (DA_PlayerControllerAsset.Succeeded())
	{
		AssetData = DA_PlayerControllerAsset.Object;
	}

	check(AssetData);

	bShowMouseCursor = true;
}

void ASMPlayerController::BeginPlay()
{
	if (!HasAuthority())
	{
		BattleHUD = Cast<USMBattleHUDWidget>(CreateWidget(this, AssetData->BattleHUDClass));
		if (BattleHUD)
		{
			BattleHUD->AddToViewport(0);
		}

		ASMGameState* SMGameState = GetWorld()->GetGameState<ASMGameState>();
		if (SMGameState)
		{
			SMGameState->OnChangeFutureBassTeamScore.AddUObject(BattleHUD, &USMBattleHUDWidget::UpdateFutureBassTeamScore);
			BattleHUD->UpdateFutureBassTeamScore(SMGameState->GetFutureBassTeamScore());
			
			SMGameState->OnChangeRockTeamScore.AddUObject(BattleHUD, &USMBattleHUDWidget::UpdateRockTeamScore);
			BattleHUD->UpdateRockTeamScore(SMGameState->GetRockTeamScore());
		}
	}

	Super::BeginPlay();
}

void ASMPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(ExitAction, ETriggerEvent::Started, this, &ASMPlayerController::Exit);
}
