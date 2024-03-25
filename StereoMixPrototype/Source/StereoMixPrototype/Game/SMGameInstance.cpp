// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SMGameInstance.h"

USMGameInstance::USMGameInstance()
{
	const int32 RandNumber = FMath::Rand();
	PlayerName = FString::Printf(TEXT("Unknown%d"), RandNumber);
}
