// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "UObject/Interface.h"
#include "SMCharacterAbilityInterface.generated.h"

class ASMPlayerCharacter;

// This class does not need to be modified.
UINTERFACE()
class USMCharacterAbilityInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class STEREOMIXPROTOTYPE_API ISMCharacterAbilityInterface
{
	GENERATED_BODY()

public:
	FORCEINLINE virtual const FName& GetAbilityName() const = 0;
	FORCEINLINE virtual ASMPlayerCharacter* GetOwnerCharacter() const = 0;
	FORCEINLINE virtual bool IsActivate() const = 0;
	FORCEINLINE virtual float GetDefaultCooldown() const = 0;
	FORCEINLINE virtual bool IsMovableAbility() const = 0;
	FORCEINLINE virtual float GetCurrentCooldown() const = 0;

	/**
	 * 어빌리티에 인풋을 바인딩합니다.
	 * @param InputComponent Component to bind input 
	 * @param InputAction  Input action to bind
	 */
	virtual void BindInput(UEnhancedInputComponent* InputComponent, const UInputAction* InputAction) = 0;

	/**
	 * 어빌리티의 오너가 초기화된 후 호출됩니다.
	 */
	virtual void PostInitializeOwner() = 0;

	/**
	 * 인풋을 입력받았을 때 호출됩니다.
	 */
	virtual void UseAbility() = 0;

	/**
	 * 어빌리티를 실행할 수 있는지 여부를 반환합니다.
	 * @return 액션 실행 가능 여부
	 */
	virtual bool CanAction() const = 0;

	/**
	 * 어빌리티가 실행 중인 경우 취소합니다.
	 * @remarks 로컬 클라이언트에서 호출된 경우에는 서버로 RPC 전송, 서버에서 호출된 경우에는 서버에서 바로 처리합니다.
	 * @return 정상적으로 취소가 됐는지 여부
	 */
	virtual void CancelImmediate() = 0;
};
