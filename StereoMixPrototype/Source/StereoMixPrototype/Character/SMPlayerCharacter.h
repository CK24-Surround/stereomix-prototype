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

DECLARE_LOG_CATEGORY_CLASS(LogSMPlayerCharacter, Log, All);

UENUM(BlueprintType)
enum class EPlayerCharacterState : uint8
{
	Normal,
	Caught
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

public:
	virtual void PossessedBy(AController* NewController) override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Controller() override;

protected: // Camera Section
	/** 카메라 초기세팅에 사용되는 함수 */
	void InitCamera();

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

protected: // Input Section
	/** 컨트롤러의 초기 세팅에 사용되는 함수입니다. */
	void InitCharacterControl();

	/** 캐릭터 기준으로 마우스 포인터가 가리키는 방향을 반환합니다. */
	FVector GetMousePointingDirection();

	/** GetMousePointingDirection()를 통해 얻어낸 방향으로 캐릭터를 회전시킵니다. */
	void UpdateRotateToMousePointer();

public: // Movement Section
	void SetEnableMovement(bool bInEnableMovement);

protected:
	/** 이동 키 입력를 실제 캐릭터 이동으로 변환해주는 함수입니다. */
	void Move(const FInputActionValue& InputActionValue);

	UFUNCTION()
	void OnRep_bEnableMovement();

	UPROPERTY(ReplicatedUsing = OnRep_bEnableMovement)
	uint32 bEnableMovement:1;

public: // State Section
	void SetCurrentState(EPlayerCharacterState InState);
	void SetEnableCollision(bool bInEnableCollision);

protected:
	UFUNCTION()
	void OnRep_CurrentState();

	UFUNCTION()
	void OnRep_bEnableCollision();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentState)
	EPlayerCharacterState CurrentState;

	UPROPERTY(ReplicatedUsing = OnRep_bEnableCollision)
	uint32 bEnableCollision:1;

public: // Control Section
	void SetCanControl(bool bInEnableControl);

protected:
	UFUNCTION()
	void OnRep_bCanControl();

	UPROPERTY()
	TObjectPtr<ASMPlayerController> StoredSMPlayerController;

	UPROPERTY(ReplicatedUsing = OnRep_bCanControl)
	uint32 bCanControl:1;

protected: // Aim Section
	UPROPERTY()
	TObjectPtr<AAimPlane> AimPlane;

protected: // Jump Section
	/** 점프 시 호출되는 이벤트입니다 */
	virtual void OnJumped_Implementation() override;

	/** 착지 시 호출되는 이벤트입니다 */
	virtual void Landed(const FHitResult& Hit) override;

protected: // Stat Section
	const float MoveSpeed = 700.0f;

protected: // Util Section
	/** 현재 캐릭터의 위치에서 가장 가까운 바닥까지의 거리를 반환합니다 */
	float DistanceHeightFromFloor();

protected: // Catch Section
	struct FPullData
	{
		AActor* Caster;
		FVector StartLocation;
		FVector EndLocation;
		uint32 bIsPulling = false;
		float ElapsedTime = 0.0f;
		float TotalTime = 0.1f;
	};

	void Catch();

	void HandleCatch();

	UFUNCTION(Server, Reliable)
	void ServerRPCPerformPull(ASMPlayerCharacter* InTargetCharacter);

	void UpdatePerformPull(float DeltaSeconds);

	void HandlePullEnd();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCAttachToCaster(AActor* InCaster, AActor* InTarget);

	FPullData PullData;

protected: // Animation Section
	UFUNCTION(Server, Unreliable)
	void ServerRPCPlayCatchAnimation() const;

	UFUNCTION(Client, Unreliable)
	void ClientRPCPlayCatchAnimation(const ASMPlayerCharacter* InPlayAnimationCharacter) const;

	UPROPERTY()
	TObjectPtr<USMCharacterAnimInstance> StoredSMAnimInstance;

public: // Animation Interface Section
	FORCEINLINE virtual bool GetHasAcceleration() override { return GetCharacterMovement()->GetCurrentAcceleration() != FVector::ZeroVector; }
	FORCEINLINE virtual bool GetIsFalling() override { return GetCharacterMovement()->IsFalling(); }
	FORCEINLINE virtual float GetZVelocity() override { return GetCharacterMovement()->Velocity.Z; }
};
