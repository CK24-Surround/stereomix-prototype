// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SMPlayerCharacterDesignData.generated.h"

/**
 * 
 */
UCLASS()
class STEREOMIXPROTOTYPE_API USMPlayerCharacterDesignData : public UDataAsset
{
	GENERATED_BODY()

// ~Attack Section
public:
	UPROPERTY(EditDefaultsOnly, Category = "Attack", DisplayName = "초당 원거리 공격속도")
	float RangedAttackFiringRate = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", DisplayName = "원거리 공격 투사체 속도")
	float RangedAttackProjectileSpeed = 3000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", DisplayName = "원거리 공격 사거리")
	float RangedAttackMaxDistance = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack" , DisplayName = "잡기 쿨타임")
	float CatchCoolDownTime = 3.0f;
// ~End of Attack Section

// ~State Section
public:
	UPROPERTY(EditDefaultsOnly, Category = "State", DisplayName = "기절 지속 시간")
	float StunTime = 6.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "State", DisplayName = "잡기 피격 시 당겨져 가는 시간")
	float CatchTime = 0.25f;
	
	UPROPERTY(EditDefaultsOnly, Category = "State", DisplayName = "매쳐진 후 기상까지 소요 시간")
	float StandUpTime = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "State", DisplayName = "팀 잡기 허용")
	uint32 bCanTeamCatch:1 = false;
// ~End of State Section
};
