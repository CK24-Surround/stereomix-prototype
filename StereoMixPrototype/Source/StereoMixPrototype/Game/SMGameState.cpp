#include "SMGameState.h"

#include "EngineUtils.h"
#include "SMGameMode.h"
#include "Log/SMLog.h"
#include "Net/UnrealNetwork.h"
#include "Tile/SMTile.h"

ASMGameState::ASMGameState() {}

void ASMGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		for (ASMTile* Tile : TActorRange<ASMTile>(GetWorld()))
		{
			if (Tile)
			{
				Tile->OnChangeTile.AddUObject(this, &ASMGameState::OnChangeTile);
			}
		}
	}

	ASMGameMode* SMGameMode = GetWorld()->GetAuthGameMode<ASMGameMode>();
	if (SMGameMode)
	{
		SMGameMode->OnChangeRemainRoundTime.BindUObject(this, &ASMGameState::SetRemainRoundTime);
		SMGameMode->OnTriggerResult.AddUObject(this, &ASMGameState::MulticastRPCProcessResult);
	}
}

void ASMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASMGameState, FutureBassTeamScore);
	DOREPLIFETIME(ASMGameState, RockTeamScore);
	DOREPLIFETIME(ASMGameState, RemainRoundTime);
}

void ASMGameState::SetFutureBassTeamScore(int32 InFutureBassTeamScore)
{
	if (HasAuthority())
	{
		if (FutureBassTeamScore == InFutureBassTeamScore)
		{
			return;
		}

		FutureBassTeamScore = InFutureBassTeamScore;

		OnRep_FutureBassTeamScore();
	}
}

void ASMGameState::SetRockTeamScore(int32 InRockTeamScore)
{
	if (HasAuthority())
	{
		if (RockTeamScore == InRockTeamScore)
		{
			return;
		}

		RockTeamScore = InRockTeamScore;

		OnRep_FutureBassTeamScore();
	}
}

void ASMGameState::OnRep_FutureBassTeamScore()
{
	OnChangeFutureBassTeamScore.Broadcast(FutureBassTeamScore);
}

void ASMGameState::OnRep_RockTeamScore()
{
	OnChangeRockTeamScore.Broadcast(RockTeamScore);
}

void ASMGameState::OnChangeTile(ESMTeam InPreviousTeam, ESMTeam InCurrentTeam)
{
	if (InPreviousTeam == InCurrentTeam)
	{
		return;
	}

	InPreviousTeam == ESMTeam::None ? AddTile(InCurrentTeam) : SwapTile(InCurrentTeam);
}

void ASMGameState::SwapTile(ESMTeam InCurrentTeam)
{
	switch (InCurrentTeam)
	{
		case ESMTeam::None:
			break;
		case ESMTeam::FutureBass:
		{
			SetRockTeamScore(RockTeamScore - 1);
			SetFutureBassTeamScore(FutureBassTeamScore + 1);
			break;
		}
		case ESMTeam::Rock:
		{
			SetFutureBassTeamScore(FutureBassTeamScore - 1);
			SetRockTeamScore(RockTeamScore + 1);
			break;
		}
	}
}

void ASMGameState::AddTile(ESMTeam InCurrentTeam)
{
	switch (InCurrentTeam)
	{
		case ESMTeam::None:
			break;
		case ESMTeam::FutureBass:
		{
			SetFutureBassTeamScore(FutureBassTeamScore + 1);
			break;
		}
		case ESMTeam::Rock:
		{
			SetRockTeamScore(RockTeamScore + 1);
			break;
		}
	}
}

void ASMGameState::SetRemainRoundTime()
{
	ASMGameMode* SMGameMode = GetWorld()->GetAuthGameMode<ASMGameMode>();
	if (SMGameMode)
	{
		RemainRoundTime = SMGameMode->GetRemainRoundTime();
	}
}

void ASMGameState::OnRep_RemainRoundTime()
{
	OnChangeRoundTime.Broadcast(RemainRoundTime);
}

void ASMGameState::MulticastRPCProcessResult_Implementation()
{
	// UI는 클라이언트 측에만 띄워주면 됩니다.
	if (!HasAuthority())
	{
		ProcessResult();
	}
}

void ASMGameState::ProcessResult()
{
	if (FutureBassTeamScore > RockTeamScore)
	{
		OnResult.Broadcast(ESMTeam::FutureBass);
	}
	else if (FutureBassTeamScore < RockTeamScore)
	{
		OnResult.Broadcast(ESMTeam::Rock);
	}
	else
	{
		OnResult.Broadcast(ESMTeam::None);
	}
}
