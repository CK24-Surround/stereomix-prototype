// ...
#pragma once

#include <concepts>
#include "CoreMinimal.h"
#include "Character/SMPlayerCharacter.h"
#include "Components/ActorComponent.h"
#include "Interface/SMCharacterAbilityInterface.h"
#include "SMCharacterAbilityComponent.generated.h"

#define DECLARE_LOG_ABILITY_CATEGORY_CLASS(AbilityName)\
	DECLARE_LOG_CATEGORY_CLASS(LogSMAbility_##AbilityName, Log, All)

template <class T>
concept sm_ability = requires(T AbilityType)
{
	{ AbilityType.UseAbility() } -> std::same_as<void>;
};

UCLASS(ClassGroup=(SMAbility), Blueprintable, meta=(BlueprintSpawnableComponent))
class STEREOMIXPROTOTYPE_API USMCharacterAbilityComponent : public UActorComponent, public ISMCharacterAbilityInterface
{
	GENERATED_BODY()

	/**
	 * 액션의 소유자 캐릭터
	 */
	UPROPERTY(Replicated, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ASMPlayerCharacter> OwnerCharacter;

	/**
	 * 액션이 현재 실행 중인지 여부를 나타냅니다.
	 */
	UPROPERTY(Transient)
	uint8 bIsActivate : 1; // 반응성을 위해 레플리케이션되지 않고 몽타쥬가 종료될 때 바로 변경됩니다.


	FOnMontageEnded OnActionMontageEndedDelegate;

protected:
	/**
	 * 어빌리티 이름
	 */
	UPROPERTY(EditDefaultsOnly, Category="Ability", meta = (AllowPrivateAccess = "true"))
	FName AbilityName;

	/**
	 * 액셔을 실행하기 위한 트리거 이벤트
	 */
	UPROPERTY(EditDefaultsOnly, Category="Ability", meta = (AllowPrivateAccess = "true"))
	ETriggerEvent TriggerOption;

	/**
	 * 액션의 쿨타임
	 */
	UPROPERTY(EditDefaultsOnly, Category="Ability", meta = (AllowPrivateAccess = "true"))
	float DefaultCooldown;

	/**
	 * 액션의 실행 딜레이
	 */
	UPROPERTY(EditDefaultsOnly, Category="Ability", meta = (AllowPrivateAccess = "true"))
	float PerformDelay;

	/**
	 * 액션이 이동 중에 실행 가능한지 여부를 나타냅니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Ability", meta = (AllowPrivateAccess = "true"))
	bool bIsMovableAbility;

	UPROPERTY(EditDefaultsOnly, Category="Ability", meta = (AllowPrivateAccess = "true"))
	float CooldownTickRate;

	/**
	 * 현재 쿨타임
	 */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_CurrentCooldown, meta = (AllowPrivateAccess = "true"))
	float CurrentCooldown;

	UPROPERTY(EditDefaultsOnly, Category="Ability", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> ActionMontage;

	// ~ISMCharacterAbilityInterface Section
public:
	FORCEINLINE virtual const FName& GetAbilityName() const override { return AbilityName; }

	FORCEINLINE virtual ASMPlayerCharacter* GetOwnerCharacter() const override { return OwnerCharacter; }

	FORCEINLINE virtual bool IsActivate() const override { return bIsActivate; }

	FORCEINLINE virtual bool IsMovableAbility() const override { return bIsMovableAbility; }

	FORCEINLINE virtual float GetDefaultCooldown() const override { return DefaultCooldown; }

	FORCEINLINE virtual float GetCurrentCooldown() const override { return CurrentCooldown; }

	virtual void BindInput(UEnhancedInputComponent* InputComponent, const UInputAction* InputAction) override;

	virtual void PostInitializeOwner() override;

	/**
	 * 클라이언트에서 인풋 입력을 받았을 때 호출되는 함수입니다.
	 * @remarks 로컬 클라이언트에서만 인풋을 입력받았을 때 호출됩니다.
	 */
	virtual void UseAbility() override;

	/**
	 * 액션을 실행할 수 있는지 여부를 반환합니다.
	 * @remarks 클라이언트, 서버 모두 액션 실행 전 호출되며 클라이언트에서 성공하더라도 서버에서 실패할 수 있습니다.
	 * @return 액션 실행 가능 여부
	 */
	UFUNCTION()
	virtual bool CanAction() const override;

	UFUNCTION()
	virtual void CancelImmediate() override;

	// ~End of ISMCharacterAbilityInterface Section

public:
	USMCharacterAbilityComponent();

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/**
	 * 서버에서 호출되는 액션 실행 RPC입니다.
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPerformAbility(const double Timestamp);

	UFUNCTION(Server, Reliable)
	void ServerCancelAbility();

	UFUNCTION()
	virtual void OnRep_IsActivate();

	UFUNCTION()
	virtual void OnRep_CurrentCooldown();

	UFUNCTION()
	virtual void OnServerPerformAbility(const double ClientDelay);

	/**
	 * 어빌리티가 끝났을 때 호출됩니다. 어빌리티가 도중에 취소됐을 때도 호출됩니다.
	 * 
	 * @remarks 서버와 클라이언트 모두에서 호출됩니다.
	 * @param bIsCanceled 액션이 취소되었는지 여부 
	 */
	UFUNCTION()
	virtual void OnEndAbility(bool bIsCanceled);

	/**
	 * 어빌리티가 취소됐을 때 호출됩니다.
	 *
	 * @remarks 서버와 클라이언트 모두에서 호출됩니다.
	 */
	UFUNCTION()
	virtual void OnCancelAbility();

	/**
	 * 어빌리티 실행 시 재생될 몽타쥬입니다.
	 * @return 
	 */
	UFUNCTION()
	virtual UAnimMontage* GetActionMontage() const { return ActionMontage; }

	/**
	 * 액션 시작 시 클라이언트에서 호출되는 RPC입니다.
	 * 이펙트 재생 등 화면에 보이는 요소들을 처리합니다.
	 * 
	 * @remarks Autonomous Proxy인 경우 액션 시작 시 바로 로컬에서 호출되며 Simulated Proxy는 서버에 의해 호출됩니다.
	 * 로컬에서는 액션이 실행됐지만 서버에서 실행되지 않은 경우 Simulated Proxy에서는 호출되지 않습니다.
	 */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayVisualEffects();

	/**
	 * 어빌리티가 시작될 때 호출됩니다. 이 함수에서 화면에 보이는 요소들을 재생합니다.
	 *
	 * @remarks 로컬에서 우선 호출 된 후 각 리모트 클라이언트에서 호출됩니다.
	 */
	UFUNCTION()
	virtual void OnPlayVisualEffectsOnAction() const;

	/**
	 * 액션 실행 시 서버에 의해 클라이언트에서 호출되는 RPC입니다.
	 * 몽타쥬 실행, 이펙트 재생 화면에 보이는 요소들을 처리합니다.
	 * 무조건 서버에 의해 클라이언트에서 호출되며 실제로 액션이 실행되었을 때 호출됩니다.
	 */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayVisualEffectsOnPerform();

	UFUNCTION()
	virtual void OnPlayVisualEffectsOnPerform() const;

	virtual void UpdateCooldownOnTick(float DeltaTime);

private:
	void OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
