// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SMProjectileAssetData.generated.h"

class UNiagaraSystem;

/**
 * 
 */
UCLASS()
class STEREOMIXPROTOTYPE_API USMProjectileAssetData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	TObjectPtr<UStaticMesh> BaseMesh;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Material")
	TObjectPtr<UMaterialInterface> FutureBassTeamMaterial;
	
	UPROPERTY(EditDefaultsOnly, Category = "Material")
	TObjectPtr<UMaterialInterface> RockTeamMaterial;
};
