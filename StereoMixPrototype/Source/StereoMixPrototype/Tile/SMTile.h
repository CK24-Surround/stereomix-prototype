// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/SMTeam.h"
#include "GameFramework/Actor.h"
#include "Interface/SMTileInterface.h"
#include "SMTile.generated.h"

class USMTileAssetData;

DECLARE_LOG_CATEGORY_CLASS(LogSMTile, Log, All);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnChangeTile, ESMTeam /*PreviousTeam*/, ESMTeam /*InCurrentTeam*/)

UCLASS()
class STEREOMIXPROTOTYPE_API ASMTile : public AActor,
                                       public ISMTileInterface
{
	GENERATED_BODY()

public:
	ASMTile();

protected:
	virtual void BeginPlay() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void TriggerTile(ESMTeam InTeam) override;

	void TileVisualChange(ESMTeam InTeam);

	TArray<ASMTile*> SelectAdjacentTiles();

protected:
	UPROPERTY()
	TObjectPtr<USMTileAssetData> AssetData;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Bottom")
	TObjectPtr<UStaticMeshComponent> BottomMesh;

	UPROPERTY(VisibleAnywhere, Category = "Frame")
	TObjectPtr<UStaticMeshComponent> FrameMesh;

	UPROPERTY(VisibleAnywhere, Category = "Tile")
	TObjectPtr<UStaticMeshComponent> TileMesh;

public:
	FORCEINLINE void SetCurrentTeam(ESMTeam InTeam)
	{
		if (HasAuthority())
		{
			CurrentTeam = InTeam;
			OnRep_CurrentTeam();
		}
	}

protected:
	UFUNCTION()
	void OnRep_CurrentTeam();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentTeam)
	ESMTeam CurrentTeam;

public:
	FOnChangeTile OnChangeTile;
};
