// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SMCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interface/SMCharacterAnimationInterface.h"
#include "Interface/SMPlayerControllerInterface.h"
#include "Interface/SMProjectileInterface.h"
#include "Interface/SMTeamComponentInterface.h"
#include "SMPlayerCharacter.generated.h"

class ASMPlayerState;
class UNiagaraComponent;
class USphereComponent;
class USMSmashComponent;
class USMPostureGaugeWidget;
class UWidgetComponent;
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
	Stun,
	Caught,
	Down,
	Smash
};

/**
 * 
 */
UCLASS()
class STEREOMIXPROTOTYPE_API ASMPlayerCharacter : public ASMCharacterBase,
                                                  public ISMTeamComponentInterface,
                                                  public ISMPlayerControllerInterface
{
	GENERATED_BODY()

public:
	explicit ASMPlayerCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	/**
	 * 캐릭터의 컨트롤러입니다.
	 * @remarks 서버에서는 모든 캐릭터가 각자의 컨트롤러를 가지고 있고, 클라이언트에서는 로컬 캐릭터만 본인의 컨트롤러를 가지고 있습니다.
	 * 그래서 리모트 클라이언트는 null입니다.
	 */
	UPROPERTY()
	TObjectPtr<ASMPlayerController> SMPlayerController;

	// ~Property Replicate Section
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentState)
	EPlayerCharacterState CurrentState;

	virtual void OnRep_PlayerState() override;

	UFUNCTION()
	void OnRep_CurrentState(const EPlayerCharacterState& OldState) const;

	// ~End of Property Replicate Section

	// ~Camera Section
protected:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	/** 카메라 초기세팅에 사용되는 함수입니다. */
	void InitCamera() const;
	// ~End of Camera Section

	// ~Input Section
protected:
	/** 컨트롤러의 초기 세팅에 사용되는 함수입니다. */
	void InitializeInputSystem() const;
	// ~End of Input Section

	// ~Movement Section
protected:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> MoveTrailEffectComponent;

	void Move(const FInputActionValue& InputActionValue);

	/** GetMousePointingDirection()를 통해 얻어낸 방향으로 캐릭터를 회전시킵니다. */
	void UpdateRotateToMousePointer();

	// ~End of Movement Section

	// ~Character State Section
public:
	FORCEINLINE EPlayerCharacterState GetCurrentState() const { return CurrentState; }
	void SetCurrentState(EPlayerCharacterState InState);

protected:
	UPROPERTY(VisibleAnywhere, Category = "HitBox")
	TObjectPtr<USphereComponent> HitBoxComponent;

	UPROPERTY()
	TObjectPtr<ASMPlayerState> SMPlayerState;
	// ~End of Character State Section

	// ~Aim Section
protected:
	UPROPERTY()
	TObjectPtr<AAimPlane> AimPlane;
	// ~End of Aim Section

	// ~Util Section
public:
	/** 서버와 시전자를 제외한 플레이어 캐릭터들을 반환합니다. */
	TArray<ASMPlayerCharacter*> GetCharactersExcludingServerAndCaster() const;

	/** 현재 캐릭터의 위치에서 가장 가까운 바닥까지의 거리를 반환합니다. */
	float DistanceHeightFromFloor() const;
	// ~End of Util Section

	// ~UI Section
protected:
	UPROPERTY(VisibleAnywhere, Category = "UI")
	TObjectPtr<UWidgetComponent> PostureGauge;

	UPROPERTY()
	TObjectPtr<USMPostureGaugeWidget> PostureGaugeWidget;
	// ~End of UI Section

	// ~Team Section
public:
	FORCEINLINE virtual USMTeamComponent* GetTeamComponent() override { return TeamComponent; }

	virtual void ResetTeamMaterial() override;

	virtual ESMTeam GetCurrentTeam() const override;

protected:
	UFUNCTION(Server, Reliable)
	void ServerRPCFutureBassTeamSelect();

	UFUNCTION(Server, Reliable)
	void ServerRPCRockTeamSelect();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Team")
	TObjectPtr<USMTeamComponent> TeamComponent;
	// ~End of Team Section

	// ~Tile Flip Section
protected:
	UPROPERTY(VisibleAnywhere, Category = "Smash")
	TObjectPtr<USMSmashComponent> SmashComponent;
	// ~End of Tile Flip Section
};
