// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SMCharacterBase.h"
#include "SMPlayerCharacter.generated.h"

class AAimPlane;
class ASMPlayerController;
class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_CLASS(LogSMPlayerCharacter, Log, All);
/**
 * 
 */
UCLASS()
class STEREOMIXPROTOTYPE_API ASMPlayerCharacter : public ASMCharacterBase
{
	GENERATED_BODY()
	
public:
	ASMPlayerCharacter();

public:
	virtual void PossessedBy(AController* NewController) override;

protected:
	virtual void BeginPlay() override;
	
public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;

protected: // Camera Section
	void InitCamera();
	
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

protected: // Move Section
	void Move(const FInputActionValue& InputActionValue);
	
	void SetCharacterControl();
	FVector GetMousePointingDirection();
	void RotateToMousePointer();

	UFUNCTION(Server, Unreliable)
	void ServerRotateToMousePointer(float InYaw);

	void OnRep_ReplicatedMovement() override;

public: // Control Section
	void SetCanControl(bool bNewCanControl);

protected: // ...
	virtual void OnRep_Controller() override;
	
	UPROPERTY()
	TObjectPtr<AAimPlane> AimPlane;
	
	UPROPERTY()
	TObjectPtr<ASMPlayerController> StoredSMPlayerController;
	
	uint32 bCanControl = true;

protected: // Jump Section
	virtual void OnJumped_Implementation() override;
	virtual void Landed(const FHitResult& Hit) override;

protected: // Stat Section
	const float MoveSpeed = 700.0f;

protected: // Util Section
	float DistanceHeightFromFloor();

protected: // Hold Section
	void Hold();
	void HandleHold();

	UFUNCTION(Server, Reliable)
	void ServerHandleHold(ASMPlayerCharacter* OtherPlayerCharacter);

	UFUNCTION(Client, Reliable)
	void ClientHandleHold();

	void HandlePulling(float DeltaSeconds);

	struct FPullData
	{
		uint32 bIsPulling = false;
		FVector StartLocation;
		FVector TargetLocation;
		float ElapsedTime = 0.0f;
		float EndTime = 1.0f;
	};
	
	FPullData PullData;
};
