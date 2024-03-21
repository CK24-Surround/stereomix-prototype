#include "SMGameState.h"

#include "EngineUtils.h"
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
}

void ASMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASMGameState, FutureBassTeamScore);
	DOREPLIFETIME(ASMGameState, RockTeamScore);
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
