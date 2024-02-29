// Fill out your copyright notice in the Description page of Project Settings.


#include "SMPlayerCharacter.h"

#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SMCharacterAssetData.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Log/SMLog.h"
#include "Physics/SMCollision.h"
#include "Player/SMPlayerController.h"

ASMPlayerCharacter::ASMPlayerCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom);

	InitCamera();

	AimPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AimPlane"));
	AimPlane->SetupAttachment(GetRootComponent());
	AimPlane->SetCollisionProfileName(CP_AIM_PLANE);
	AimPlane->SetRelativeScale3D(FVector(100.0));
	AimPlane->SetVisibility(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshRef(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
	if (PlaneMeshRef.Succeeded())
	{
		AimPlane->SetStaticMesh(PlaneMeshRef.Object);
	}

	bUseControllerRotationYaw = false;
}

void ASMPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	StoredSMPlayerController = CastChecked<ASMPlayerController>(GetController());
}

void ASMPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetCharacterControl();
}

void ASMPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(AssetData->MoveAction, ETriggerEvent::Triggered, this, &ASMPlayerCharacter::Move);
		EnhancedInputComponent->BindAction(AssetData->JumpAction, ETriggerEvent::Triggered, this, &ASMPlayerCharacter::Jump);
	}
}

void ASMPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	RotateToMousePointer();
}

void ASMPlayerCharacter::InitCamera()
{
	const FRotator CameraRotation(-45.0f, 0.0, 0.0);
	const float CameraDistance = 750.0f;
	const float CameraFOV = 90.0f;

	CameraBoom->SetRelativeRotation(CameraRotation);
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->TargetArmLength = CameraDistance;
	CameraBoom->bDoCollisionTest = false;

	Camera->SetFieldOfView(CameraFOV);
}

void ASMPlayerCharacter::SetCharacterControl()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (Subsystem)
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(AssetData->DefaultMappingContext, 0);
		}
	}

	FInputModeGameOnly InputModeGameOnly;
	InputModeGameOnly.SetConsumeCaptureMouseDown(false);
	PlayerController->SetInputMode(InputModeGameOnly);
}

FVector ASMPlayerCharacter::GetMousePointingDirection()
{
	if (StoredSMPlayerController)
	{
		FHitResult HitResult;
		const bool Succeed = StoredSMPlayerController->GetHitResultUnderCursor(TC_AIM_PLANE, false, HitResult);

		if (Succeed)
		{
			const FVector MouseLocation = HitResult.Location;
			const FVector MouseDirection = (MouseLocation - GetActorLocation()).GetSafeNormal();
			return MouseDirection;
		}
	}

	return FVector::ZeroVector;
}

void ASMPlayerCharacter::RotateToMousePointer()
{
	if (IsLocallyControlled())
	{
		const FRotator NewRotator = FRotator(0.0, FRotationMatrix::MakeFromX(GetMousePointingDirection()).Rotator().Yaw, 0.0);
		
		SetActorRotation(NewRotator);

		if (!HasAuthority())
		{
			ServerRotateToMousePointer(NewRotator.Yaw);
		}
	}
}

void ASMPlayerCharacter::ServerRotateToMousePointer_Implementation(float InYaw)
{
	// 트랜스폼은 리플리케이트 무브먼트 활성화 시 자동으로 모든 클라이언트에 동기화하기 때문에 서버에서만 처리해주면 됩니다.
	const FRotator NewRotator = FMath::RInterpTo(GetActorRotation(), FRotator(0.0, InYaw, 0.0), GetWorld()->GetDeltaSeconds(), 50.0f);
	SetActorRotation(NewRotator);
}

void ASMPlayerCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	if (IsLocallyControlled())
	{
		StoredSMPlayerController = CastChecked<ASMPlayerController>(GetController());
	}
}

void ASMPlayerCharacter::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputScalar = InputActionValue.Get<FVector2D>().GetSafeNormal();
	const FRotator CameraYawRotation(0.0, Camera->GetComponentRotation().Yaw, 0.0);
	const FVector ForwardDirection = FRotationMatrix(CameraYawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(CameraYawRotation).GetUnitAxis(EAxis::Y);
	const FVector MoveVector = (ForwardDirection * InputScalar.X) + (RightDirection * InputScalar.Y);

	AddMovementInput(MoveVector);
}
