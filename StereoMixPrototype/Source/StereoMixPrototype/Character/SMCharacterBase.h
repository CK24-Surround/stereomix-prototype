// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SMCharacterBase.generated.h"

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

protected: // Data Section
	virtual void CheckAssetLoaded();

	UPROPERTY()
	TObjectPtr<USMCharacterAssetData> AssetData;
};
