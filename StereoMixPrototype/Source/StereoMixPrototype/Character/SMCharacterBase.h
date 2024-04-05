// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SMCharacterBase.generated.h"

class USMPlayerCharacterDesignData;
class USMCharacterStatComponent;
class USMCharacterAssetData;
class USMCharacterAbilityManagerComponent;

UCLASS()
class STEREOMIXPROTOTYPE_API ASMCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ASMCharacterBase(const FObjectInitializer& ObjectInitializer);

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

	UPROPERTY(Category = "Ability", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<USMCharacterAbilityManagerComponent> AbilityManager;

public:
	FORCEINLINE USMCharacterStatComponent* GetStat() const { return Stat; }
	FORCEINLINE USMCharacterAbilityManagerComponent* GetAbilityManager() const { return AbilityManager; }
	// ~End of Component Section
};
