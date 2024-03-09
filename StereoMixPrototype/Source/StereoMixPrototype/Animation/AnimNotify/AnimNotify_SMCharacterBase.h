// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SMCharacterBase.generated.h"

class ISMCharacterAnimationInterface;
/**
 * 
 */
UCLASS(Abstract)
class STEREOMIXPROTOTYPE_API UAnimNotify_SMCharacterBase : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	ISMCharacterAnimationInterface* StoredAnimationInterface;
};
