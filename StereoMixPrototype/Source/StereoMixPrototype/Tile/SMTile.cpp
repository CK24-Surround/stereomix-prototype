// Fill out your copyright notice in the Description page of Project Settings.


#include "SMTile.h"

ASMTile::ASMTile()
{
	PrimaryActorTick.bCanEverTick = false;
	
	BottomMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BottomMesh"));
	SetRootComponent(BottomMesh);

	FrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameMesh"));
	FrameMesh->SetupAttachment(BottomMesh);
	
	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	TileMesh->SetupAttachment(FrameMesh);
}

void ASMTile::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASMTile::TileChange()
{
	
}

