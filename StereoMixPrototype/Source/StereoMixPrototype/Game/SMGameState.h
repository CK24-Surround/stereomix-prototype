

#pragma once

#include "CoreMinimal.h"
#include "Data/SMTeam.h"
#include "GameFramework/GameState.h"
#include "SMGameState.generated.h"

class ASMTile;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnChangeScore, int32 /*CurrentScore*/);

/**
 * 
 */
UCLASS()
class STEREOMIXPROTOTYPE_API ASMGameState : public AGameState
{
	GENERATED_BODY()

public:
	ASMGameState();
	virtual void PostInitializeComponents() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	FORCEINLINE int32 GetFutureBassTeamScore() const { return FutureBassTeamScore; }
	void SetFutureBassTeamScore(int32 InFutureBassTeamScore);

	FORCEINLINE int32 GetRockTeamScore() const { return RockTeamScore; }
	void SetRockTeamScore(int32 InRockTeamScore);
	
protected:
	UFUNCTION()
	void OnRep_FutureBassTeamScore();

	UFUNCTION()
	void OnRep_RockTeamScore();
	
protected:
	UPROPERTY(ReplicatedUsing = OnRep_FutureBassTeamScore)
	int32 FutureBassTeamScore = 0;

	UPROPERTY(ReplicatedUsing = OnRep_RockTeamScore)
	int32 RockTeamScore = 0;

public:
	FOnChangeScore OnChangeFutureBassTeamScore;
	FOnChangeScore OnChangeRockTeamScore;

// ~Tile Section
protected:
	TArray<TSoftObjectPtr<ASMTile>> Tiles;

protected:
	void OnChangeTile(ESMTeam InPreviousTeam, ESMTeam InCurrentTeam);

	void SwapTile(ESMTeam InCurrentTeam);
	
	void AddTile(ESMTeam InCurrentTeam);
// ~End of Tile Section
};
