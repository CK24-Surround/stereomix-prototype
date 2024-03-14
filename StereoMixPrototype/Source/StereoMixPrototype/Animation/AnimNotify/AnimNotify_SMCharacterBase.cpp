// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/AnimNotify_SMCharacterBase.h"

#include "Interface/SMCharacterAnimationInterface.h"

void UAnimNotify_SMCharacterBase::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	StoredAnimationInterface = Cast<ISMCharacterAnimationInterface>(MeshComp->GetOwner());
}