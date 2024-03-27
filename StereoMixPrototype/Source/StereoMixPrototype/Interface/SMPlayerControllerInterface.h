// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/SMTeam.h"
#include "UObject/Interface.h"
#include "SMPlayerControllerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class USMPlayerControllerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class STEREOMIXPROTOTYPE_API ISMPlayerControllerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual ESMTeam GetCurrentTeam() const = 0;
};