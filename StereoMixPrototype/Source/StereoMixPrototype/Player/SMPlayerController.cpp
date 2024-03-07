// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SMPlayerController.h"

#include "EnhancedInputComponent.h"

ASMPlayerController::ASMPlayerController()
{
	bShowMouseCursor = true;
}

void ASMPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ASMPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(ExitAction, ETriggerEvent::Started, this, &ASMPlayerController::Exit);
}
