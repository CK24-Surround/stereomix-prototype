// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SMCharacterBase.h"

// Sets default values
ASMCharacterBase::ASMCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

// Called to bind functionality to input
void ASMCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

