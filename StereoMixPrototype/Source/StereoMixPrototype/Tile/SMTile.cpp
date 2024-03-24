// Fill out your copyright notice in the Description page of Project Settings.


#include "SMTile.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "SMTileAssetData.h"
#include "Data/AssetPath.h"
#include "Log/SMLog.h"
#include "Net/UnrealNetwork.h"
#include "Physics/SMCollision.h"

ASMTile::ASMTile()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<USMTileAssetData> DA_TileAsset(TILE_DATA_ASSET_PATH);
	if (DA_TileAsset.Succeeded())
	{
		AssetData = DA_TileAsset.Object;
	}

	check(AssetData);

	BottomMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BottomMesh"));
	SetRootComponent(BottomMesh);
	BottomMesh->SetCollisionProfileName(CP_FLOOR);

	FrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameMesh"));
	FrameMesh->SetupAttachment(BottomMesh);
	FrameMesh->SetCollisionProfileName(CP_FLOOR);

	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	TileMesh->SetupAttachment(FrameMesh);
	TileMesh->SetCollisionProfileName(CP_TILE);

	SmashEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SmashEffect"));
	SmashEffectComponent->SetupAttachment(TileMesh);
	SmashEffectComponent->SetAsset(AssetData->SmashEffect);
	SmashEffectComponent->SetAutoActivate(false);

	bReplicates = true;
}

void ASMTile::BeginPlay()
{
	Super::BeginPlay();
}

void ASMTile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASMTile, CurrentTeam);
}

void ASMTile::TriggerTile(ESMTeam InTeam)
{
	// 서버에서 호출됩니다. SmashComponent에서 호출됩니다.

	NET_LOG(LogSMTile, Log, TEXT("트리거 된 타일: %s"), *GetName());
	TArray<ASMTile*> SelectedTiles = SelectAdjacentTiles();

	for (const auto& SelectedTile : SelectedTiles)
	{
		const ESMTeam PreviousTeam = SelectedTile->CurrentTeam;
		SelectedTile->SetCurrentTeam(InTeam);
		SelectedTile->OnChangeTile.Broadcast(PreviousTeam, InTeam);

		// const FString PreviousTeamName = UEnum::GetValueAsString(TEXT("StereoMixPrototype.ESMTeam"), PreviousTeam);
		// const FString CurrentTeamName = UEnum::GetValueAsString(TEXT("StereoMixPrototype.ESMTeam"), InTeam);
		// NET_LOG(LogSMNetwork, Warning, TEXT("Prev: %s, Cur: %s"), *PreviousTeamName, *CurrentTeamName);
	}

	PlaySmashEffect(this);
}

void ASMTile::PlaySmashEffect_Implementation(ASMTile* TileToPlayEffect)
{
	if (!HasAuthority())
	{
		NET_LOG(LogSMTile, Warning, TEXT("스매시 이펙트 재생"))
		SmashEffectComponent->ActivateSystem();
	}
}

void ASMTile::TileVisualChange(ESMTeam InTeam)
{
	switch (InTeam)
	{
		case ESMTeam::None:
			break;
		case ESMTeam::FutureBass:
		{
			TileMesh->SetMaterial(0, AssetData->FutureBassTeamMaterial);
			break;
		}
		case ESMTeam::Rock:
		{
			TileMesh->SetMaterial(0, AssetData->RockTeamMaterial);
			break;
		}
	}

	//TODO: 점수 처리 로직 필요
}

TArray<ASMTile*> ASMTile::SelectAdjacentTiles()
{
	auto test = FCollisionQueryParams::DefaultQueryParam;
	TArray<FOverlapResult> HitResults;
	const FVector Center = GetActorLocation() + FVector(0.0, 0.0, 100.0);
	const FVector Box(300.0f);
	const bool bSuccess = GetWorld()->OverlapMultiByChannel(HitResults, Center, FQuat::Identity, TC_TILE, FCollisionShape::MakeBox(Box));

	TArray<ASMTile*> SelectedTiles;
	if (bSuccess)
	{
		for (const auto& HitResult : HitResults)
		{
			ASMTile* SelectedTile = Cast<ASMTile>(HitResult.GetActor());
			if (SelectedTile)
			{
				SelectedTiles.Add(SelectedTile);
			}
		}
	}

	DrawDebugBox(GetWorld(), Center, Box, FColor::Cyan, false, 2.0f);
	return SelectedTiles;
}

void ASMTile::OnRep_CurrentTeam()
{
	if (!HasAuthority())
	{
		TileVisualChange(CurrentTeam);
	}
}
