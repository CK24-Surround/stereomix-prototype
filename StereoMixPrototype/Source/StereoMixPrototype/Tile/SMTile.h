// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMTile.generated.h"

UCLASS()
class STEREOMIXPROTOTYPE_API ASMTile : public AActor
{
	GENERATED_BODY()
	
public:	
	ASMTile();

protected:
	virtual void BeginPlay() override;

public:
	void TileChange();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Bottom")
	TObjectPtr<UStaticMeshComponent> BottomMesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Frame")
	TObjectPtr<UStaticMeshComponent> FrameMesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Tile")
	TObjectPtr<UStaticMeshComponent> TileMesh;
};
