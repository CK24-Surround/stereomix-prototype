// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SMCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interface/SMCharacterAnimationInterface.h"
#include "SMPlayerCharacter.generated.h"

class USMCharacterAnimInstance;
class AAimPlane;
class ASMPlayerController;
class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_CLASS(LogSMCharacter, Log, All);

UENUM(BlueprintType)
enum class EPlayerCharacterState : uint8
{
	Normal,
	Caught,
	Down
};

/**
 * 
 */
UCLASS()
class STEREOMIXPROTOTYPE_API ASMPlayerCharacter : public ASMCharacterBase, public ISMCharacterAnimationInterface
{
	GENERATED_BODY()

public:
	ASMPlayerCharacter();

public:
	virtual void PostInitializeComponents() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

// ~Property Replicate Section
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void OnRep_Controller() override;

protected:
	UFUNCTION()
	void OnRep_bCanControl();
	
	UFUNCTION()
	void OnRep_CurrentState();

	UFUNCTION()
	void OnRep_bEnableMovement();

	UFUNCTION()
	void OnRep_bEnableCollision();

	UFUNCTION()
	void OnRep_CollisionProfileName();
	
protected:
	UPROPERTY(ReplicatedUsing = OnRep_bCanControl)
	uint32 bCanControl:1;
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentState)
	EPlayerCharacterState CurrentState;
	
	UPROPERTY(ReplicatedUsing = OnRep_bEnableMovement)
	uint32 bEnableMovement:1;

	UPROPERTY(ReplicatedUsing = OnRep_bEnableCollision)
	uint32 bEnableCollision:1;

	UPROPERTY(ReplicatedUsing = OnRep_CollisionProfileName)
	FName CollisionProfileName;
// ~End of Property Replicate Section

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;

// ~Design Section
protected:
	void InitDesignData();

	UPROPERTY(EditAnywhere, Category = "Design(Catch)")
	float CatchTime;

	UPROPERTY(EditAnywhere, Category = "Design(Stand Up)")
	float StandUpTime;

	UPROPERTY(EditAnywhere, Category = "Design(Ranged Attack)")
	float RangedAttackFiringRate;
// ~End of Design Section

// ~Camera Section
protected:
	/** 카메라 초기세팅에 사용되는 함수입니다. */
	void InitCamera();

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;
// ~End of Camera Section

// ~Input Section
protected:
	/** 컨트롤러의 초기 세팅에 사용되는 함수입니다. */
	void InitCharacterControl();

	/** 캐릭터 기준으로 마우스 포인터가 가리키는 방향을 반환합니다. */
	FVector GetMousePointingDirection();

	/** GetMousePointingDirection()를 통해 얻어낸 방향으로 캐릭터를 회전시킵니다. */
	void UpdateRotateToMousePointer();
// ~End of Input Section

// ~Movement Section
public:
	void SetEnableMovement(bool bInEnableMovement);

protected:
	void Move(const FInputActionValue& InputActionValue);
// ~End of Movement Section

// ~Character State Section
public:
	void SetCurrentState(EPlayerCharacterState InState);
	void SetEnableCollision(bool bInEnableCollision);
	void SetCollisionProfileName(FName InCollisionProfileName);
// ~End of Character Section

// ~Aim Section
protected:
	UPROPERTY()
	TObjectPtr<AAimPlane> AimPlane;
// ~End of Aim Section
	
// ~Control Section
public:
	void SetCanControl(bool bInEnableControl);

protected:
	UPROPERTY()
	TObjectPtr<ASMPlayerController> StoredSMPlayerController;
// ~End of Control Section
	
// ~Util Section
public:
	/** 서버와 시전자를 제외한 플레이어 캐릭터들을 반환합니다. */
	TArray<ASMPlayerCharacter*> GetCharactersExcludingServerAndCaster();

	/** 현재 캐릭터의 위치에서 가장 가까운 바닥까지의 거리를 반환합니다. */
	float DistanceHeightFromFloor();
// ~End of Util Section

// ~Jump Section
protected:
	/** 점프 시 호출되는 이벤트입니다. */
	virtual void OnJumped_Implementation() override;

	/** 착지 시 호출되는 이벤트입니다. */
	virtual void Landed(const FHitResult& Hit) override;
// ~Jump Section

// ~Stat Section
protected:
	const float MoveSpeed = 700.0f;
// ~End of Stat Section

// ~Catch Section
protected:
	struct FPullData
	{
		uint32 bIsPulling:1 = false;
		ASMPlayerCharacter* Caster;
		FVector StartLocation;
		FVector EndLocation;
		float ElapsedTime = 0.0f;
		float TotalTime = 0.25f;
		float LastServerTime = 0.0f;
	};
	
public: 
	void SetCaughtCharacter(ASMPlayerCharacter* InCaughtCharacter);

protected:
	void Catch();

	UFUNCTION(Server, Unreliable)
	void ServerRPCPlayCatchAnimation();

	UFUNCTION(Client, Unreliable)
	void ClientRPCPlayCatchAnimation(const ASMPlayerCharacter* InPlayAnimationCharacter) const;
	
	virtual void AnimNotify_Catch() override;

	UFUNCTION(Server, Reliable)
	void ServerRPCPerformPull(ASMPlayerCharacter* InTargetCharacter);

	UFUNCTION(Client, Reliable)
	void ClientRPCLastTimeCheck();

	void UpdatePerformPull();

	UFUNCTION(Client, Unreliable)
	void ClientRPCInterpolationPull(FVector_NetQuantize10 InInterpolationLocation);

	void UpdateInterpolationPull();

	void HandlePullEnd();

	UFUNCTION(Client, Reliable)
	void ClientRPCPlayCaughtAnimation(ASMPlayerCharacter* InPlayAnimationCharacter) const;

	FPullData PullData;

	UPROPERTY()
	TObjectPtr<USMCharacterAnimInstance> StoredSMAnimInstance;

	UPROPERTY(Replicated)
	TObjectPtr<ASMPlayerCharacter> CaughtCharacter;

public: // Animation Interface Section
	FORCEINLINE virtual bool GetHasAcceleration() override { return GetCharacterMovement()->GetCurrentAcceleration() != FVector::ZeroVector; }
	FORCEINLINE virtual bool GetIsFalling() override { return GetCharacterMovement()->IsFalling(); }
	FORCEINLINE virtual float GetZVelocity() override { return GetCharacterMovement()->Velocity.Z; }

// ~Smash Section
protected:
	void Smash();

	UFUNCTION(Server, Unreliable)
	void ServerRPCPlaySmashAnimation();

	UFUNCTION(Client, Unreliable)
	void ClientRPCPlaySmashAnimation(const ASMPlayerCharacter* InCharacterToAnimation) const;

	/** 애님 노티파이에 의해 호출됩니다. */
	virtual void AnimNotify_Smash() override;

	UFUNCTION(Server, Unreliable)
	void ServerRPCPlayKnockDownAnimation();

	UFUNCTION(Client, Unreliable)
	void ClientRPCPlayKnockDownAnimation(ASMPlayerCharacter* InCharacterToAnimation);
	
	UFUNCTION(Server, Reliable)
	void ServerRPCDetachToCaster(FVector_NetQuantize10 InLocation, FRotator InRotation);

	/** 스매시 애니메이션이 종료되고난 뒤에 호출됩니다. */
	void SmashEnded(UAnimMontage* PlayAnimMontage, bool bInterrupted);

	UFUNCTION(Client, Reliable)
	void ClientRPCSetRotation(FRotator InRotation);

	/** 타이머 실행 이후(기상 시간) 기상 애니메이션을 재생시킵니다. */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCPlayStandUpAnimation();

	/** 기상 애니메이션이 종료되고난 뒤에 호출됩니다. */
	void StandUpEnded(UAnimMontage* PlayAnimMontage, bool bInterrupted);
// ~End of Smash Section

// ~Ranged Attack Section
protected:
	void RangedAttack();

	void CanRangedAttack();

	UFUNCTION(Server, Reliable)
	void ServerRPCRequestShootProjectile();

protected:
	uint32 bCanRangedAttack:1 = true;
// ~End of Ranged Attack Section
};
