// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SMCharacterBase.generated.h"

class USMPlayerCharacterDesignData;
class USMCharacterStatComponent;
class USMCharacterAssetData;

UCLASS()
class STEREOMIXPROTOTYPE_API ASMCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ASMCharacterBase();

public:
	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

// ~Data Section
protected:
	void AssetCheck();

	UPROPERTY()
	TObjectPtr<const USMCharacterAssetData> AssetData;

	UPROPERTY()
	TObjectPtr<const USMPlayerCharacterDesignData> DesignData;

// ~End of Data Section

// ~Component Section
protected:
	UPROPERTY()
	TObjectPtr<USMCharacterStatComponent> Stat;
// ~End of Component Section
};
