// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SMPlayerController.h"

#include "EnhancedInputComponent.h"
#include "SMPlayerControllerAssetData.h"
#include "Blueprint/UserWidget.h"
#include "Data/AssetPath.h"
#include "Game/SMGameState.h"
#include "Interface/SMPlayerControllerInterface.h"
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
		if (!BattleHUD)
		{
			return;
		}

		ASMGameState* SMGameState = GetWorld()->GetGameState<ASMGameState>();
		if (!SMGameState)
		{
			return;
		}

		BattleHUD->AddToViewport(0);

		SMGameState->OnChangeFutureBassTeamScore.AddUObject(BattleHUD, &USMBattleHUDWidget::UpdateFutureBassTeamScore);
		BattleHUD->UpdateFutureBassTeamScore(SMGameState->GetFutureBassTeamScore());

		SMGameState->OnChangeRockTeamScore.AddUObject(BattleHUD, &USMBattleHUDWidget::UpdateRockTeamScore);
		BattleHUD->UpdateRockTeamScore(SMGameState->GetRockTeamScore());

		SMGameState->OnChangeRoundTime.AddUObject(BattleHUD, &USMBattleHUDWidget::UpdateRemainRoundTime);
		BattleHUD->UpdateRemainRoundTime(SMGameState->GetRemainRoundTime());

		SMGameState->OnResult.AddUObject(this, &ASMPlayerController::ProcessResult);
	}

	Super::BeginPlay();
}

void ASMPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(ExitAction, ETriggerEvent::Started, this, &ASMPlayerController::Exit);
}

void ASMPlayerController::ProcessResult(ESMTeam InVictoryTeam)
{
	const ISMPlayerControllerInterface* SMPlayerControllerInterface = Cast<ISMPlayerControllerInterface>(GetPawn());
	if (SMPlayerControllerInterface)
	{
		const ESMTeam CharacterTeam = SMPlayerControllerInterface->GetCurrentTeam();
		if (!BattleHUD)
		{
			return;
		}

		if (CharacterTeam == ESMTeam::None)
		{
			NET_LOG(LogSMPlayerController, Log, TEXT("팀 없음"));
			return;
		}
		else if (InVictoryTeam == ESMTeam::None)
		{
			NET_LOG(LogSMPlayerController, Log, TEXT("무승부 트리거"));
			BattleHUD->ShowResultDraw();
		}
		else if (CharacterTeam == InVictoryTeam)
		{
			NET_LOG(LogSMPlayerController, Log, TEXT("승리 트리거"));
			BattleHUD->ShowResult(true);
		}
		else
		{
			NET_LOG(LogSMPlayerController, Log, TEXT("패배 트리거"));
			BattleHUD->ShowResult(false);
		}
	}
}