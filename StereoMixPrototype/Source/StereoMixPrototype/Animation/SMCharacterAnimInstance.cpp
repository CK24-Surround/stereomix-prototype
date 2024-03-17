// Fill out your copyright notice in the Description page of Project Settings.


#include "SMCharacterAnimInstance.h"

#include "SMCharacterAnimationAssetData.h"
#include "Data/AssetPath.h"
#include "Interface/SMCharacterAnimationInterface.h"
#include "Log/SMLog.h"

USMCharacterAnimInstance::USMCharacterAnimInstance()
{
	static ConstructorHelpers::FObjectFinder<USMCharacterAnimationAssetData> DA_CharacterAnimationAsset(CHARACTER_ANIMATION_ASSET_PATH);
	if (DA_CharacterAnimationAsset.Succeeded())
	{
		AssetData = DA_CharacterAnimationAsset.Object;
	}

	StoredAnimationInterface = nullptr;
	bHasAcceleration = false;
	bIsFalling = false;
	ZVelocity = 0.0f;
}

void USMCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	AssetCheck();

	StoredAnimationInterface = Cast<ISMCharacterAnimationInterface>(GetOwningActor());
}

void USMCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (StoredAnimationInterface)
	{
		bHasAcceleration = StoredAnimationInterface->GetHasAcceleration();
		bIsFalling = StoredAnimationInterface->GetIsFalling();
		ZVelocity = StoredAnimationInterface->GetZVelocity();
	}
}

void USMCharacterAnimInstance::AssetCheck()
{
	check(AssetData);
}

void USMCharacterAnimInstance::PlayCatch()
{
	NET_ANIM_LOG(LogSMAnim, Log, TEXT("잡기 애니메이션 재생"));
	Montage_Play(AssetData->CatchMontage);
}

void USMCharacterAnimInstance::PlayCaught()
{
	NET_ANIM_LOG(LogSMAnim, Log, TEXT("잡힌 애니메이션 재생"));
	Montage_Play(AssetData->CaughtMontage);
}

void USMCharacterAnimInstance::PlaySmash()
{
	NET_ANIM_LOG(LogSMAnim, Log, TEXT("매치기 애니메이션 재생"));
	Montage_Play(AssetData->SmashMontage);
	Montage_SetEndDelegate(OnSmashEnded, AssetData->SmashMontage);
}

void USMCharacterAnimInstance::PlayKnockDown()
{
	NET_ANIM_LOG(LogSMAnim, Log, TEXT("매쳐지기 애니메이션 재생"));
	Montage_Play(AssetData->DownStartMontage);
}

void USMCharacterAnimInstance::PlayStandUp()
{
	NET_ANIM_LOG(LogSMAnim, Log, TEXT("기상 애니메이션 재생"));
	Montage_Play(AssetData->DownEndMontage);
	Montage_SetEndDelegate(OnStandUpEnded, AssetData->DownEndMontage);
}

void USMCharacterAnimInstance::PlayRangedAttack()
{
	NET_ANIM_LOG(LogSMAnim, Log, TEXT("원거리 공격 재생"));
	Montage_Play(AssetData->RangedAttackMontage);
}

void USMCharacterAnimInstance::PlayStun()
{
	NET_ANIM_LOG(LogSMAnim, Log, TEXT("기절 애니메이션 재생"));
	Montage_Play(AssetData->Stun);
	Montage_SetEndDelegate(OnStunEnded, AssetData->Stun);
}

void USMCharacterAnimInstance::PlayStunEnd()
{
	NET_ANIM_LOG(LogSMAnim, Log, TEXT("기절 종료 애니메이션 재생"));
	if (Montage_IsPlaying(AssetData->Stun))
	{
		Montage_JumpToSection(TEXT("End"), AssetData->Stun);
	}
}

float USMCharacterAnimInstance::GetStunEndLength()
{
	float StunEndLength = 0.0f;
	if (AssetData->StunEnd)
	{
		StunEndLength = AssetData->StunEnd->GetPlayLength();
	}

	return StunEndLength;
}
