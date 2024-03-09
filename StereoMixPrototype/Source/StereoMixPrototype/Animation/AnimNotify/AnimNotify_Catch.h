// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotify/AnimNotify_SMCharacterBase.h"
#include "AnimNotify_Catch.generated.h"

/**
 * 
 */
UCLASS()
class STEREOMIXPROTOTYPE_API UAnimNotify_Catch : public UAnimNotify_SMCharacterBase
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
