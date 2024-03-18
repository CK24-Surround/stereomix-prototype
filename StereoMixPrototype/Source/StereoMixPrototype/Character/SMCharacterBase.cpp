// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SMCharacterBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SMCharacterAssetData.h"
#include "CharacterStat/SMCharacterStatComponent.h"
#include "Data/AssetPath.h"
#include "Design/SMPlayerCharacterDesignData.h"

// Sets default values
ASMCharacterBase::ASMCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<USMCharacterAssetData> SMCharacterAssetDataRef(CHARACTER_ASSET_PATH);
	if (SMCharacterAssetDataRef.Succeeded())
	{
		AssetData = SMCharacterAssetDataRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<USMPlayerCharacterDesignData> SMPlayerCharacterDesignDataRef(PLAYER_CHARACTER_DESIGN_DATA_ASSET_PATH);
	if (SMPlayerCharacterDesignDataRef.Succeeded())
	{
		DesignData = SMPlayerCharacterDesignDataRef.Object;
	}

	check(AssetData);
	check(DesignData);

	AssetCheck();
	
	Stat = CreateDefaultSubobject<USMCharacterStatComponent>(TEXT("CharacterStat"));
}

void ASMCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// Called when the game starts or when spawned
void ASMCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASMCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASMCharacterBase::AssetCheck()
{
	check(AssetData);
	check(DesignData);
}
