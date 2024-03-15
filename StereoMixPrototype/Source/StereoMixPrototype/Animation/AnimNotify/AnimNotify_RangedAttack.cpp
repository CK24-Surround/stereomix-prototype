// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/AnimNotify_RangedAttack.h"

#include "Interface/SMCharacterAnimationInterface.h"

void UAnimNotify_RangedAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (StoredAnimationInterface)
	{
		StoredAnimationInterface->AnimNotify_RangedAttack();
	}
}
