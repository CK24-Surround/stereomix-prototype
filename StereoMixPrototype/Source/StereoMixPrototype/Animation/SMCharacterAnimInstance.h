// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SMCharacterAnimInstance.generated.h"

class ISMCharacterAnimationInterface;
class USMCharacterAnimationAssetData;

DECLARE_LOG_CATEGORY_CLASS(LogSMAnim, Log, All);
/**
 * 
 */
UCLASS()
class STEREOMIXPROTOTYPE_API USMCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	USMCharacterAnimInstance();
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected: // Data Asset Section
	void AssetCheck();
	
	UPROPERTY()
	TObjectPtr<const USMCharacterAnimationAssetData> AssetData; 

protected: // Cached Section
	ISMCharacterAnimationInterface* StoredAnimationInterface;

public: // Montage Section
	void PlayCatch();
	void PlayCaught();

protected: // State Section
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterState")
	uint32 bHasAcceleration:1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterState")
	uint32 bIsFalling:1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterState")
	float ZVelocity;
};
