// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SMCharacterBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SMCharacterAssetData.h"
#include "Data/AssetPath.h"

// Sets default values
ASMCharacterBase::ASMCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<USMCharacterAssetData> SMCharacterAssetDataRef(CHARACTER_ASSET_PATH);
	if (SMCharacterAssetDataRef.Succeeded())
	{
		AssetData = SMCharacterAssetDataRef.Object;
	}
}

void ASMCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CheckAssetLoaded();
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

void ASMCharacterBase::CheckAssetLoaded()
{
	check(AssetData);
}
