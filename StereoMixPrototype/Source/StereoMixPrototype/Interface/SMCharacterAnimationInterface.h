// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SMCharacterAnimationInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USMCharacterAnimationInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class STEREOMIXPROTOTYPE_API ISMCharacterAnimationInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool GetHasAcceleration() = 0;
	virtual bool GetIsFalling() = 0;
	virtual float GetZVelocity() = 0;
};
