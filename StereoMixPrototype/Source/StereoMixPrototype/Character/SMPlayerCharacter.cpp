// Fill out your copyright notice in the Description page of Project Settings.


#include "SMPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SMCharacterAssetData.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Log/SMLog.h"
#include "Net/UnrealNetwork.h"
#include "Physics/SMCollision.h"
#include "Player/AimPlane.h"
#include "Player/SMPlayerController.h"

ASMPlayerCharacter::ASMPlayerCharacter()
{
	bUseControllerRotationYaw = false;

	GetMesh()->SetCollisionProfileName("NoCollision");

	UCharacterMovementComponent* CachedCharacterMovement = GetCharacterMovement();
	CachedCharacterMovement->MaxWalkSpeed = MoveSpeed;
	CachedCharacterMovement->MaxAcceleration = 10000.0f;
	CachedCharacterMovement->BrakingDecelerationWalking = 10000.0f;
	CachedCharacterMovement->BrakingDecelerationFalling = 0.0f; // ~ 100 중 선택
	CachedCharacterMovement->AirControl = 1.0f;

	CachedCharacterMovement->GravityScale = 2.0f;
	CachedCharacterMovement->JumpZVelocity = 700.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom);

	InitCamera();

	CurrentState = EPlayerCharacterState::Normal;
	bEnableCollision = true;
	bCanControl = true;
}

void ASMPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 무조건 서버에서만 실행되겠지만 만약을 위한 조건입니다.
	if (HasAuthority())
	{
		// 서버에선 OnRep_Controller가 호출되지 않고, 클라이언트에서는 PossessedBy가 호출되지 않기 때문에 서버는 여기서 컨트롤러를 캐싱합니다.
		StoredSMPlayerController = CastChecked<ASMPlayerController>(GetController());
	}
}

void ASMPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		AimPlane = GetWorld()->SpawnActor<AAimPlane>();
		const FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
		const bool bSuccess = AimPlane->AttachToActor(this, AttachmentTransformRules);
		if (bSuccess) {}
	}

	InitCharacterControl();
}

void ASMPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(AssetData->MoveAction, ETriggerEvent::Triggered, this, &ASMPlayerCharacter::Move);
		EnhancedInputComponent->BindAction(AssetData->JumpAction, ETriggerEvent::Triggered, this, &ASMPlayerCharacter::Jump);
		EnhancedInputComponent->BindAction(AssetData->HoldAction, ETriggerEvent::Started, this, &ASMPlayerCharacter::Catch);
	}
}

void ASMPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bCanControl)
	{
		UpdateRotateToMousePointer();
	}

	if (HasAuthority())
	{
		if (PullData.bIsPulling)
		{
			UpdatePerformPull(DeltaSeconds);
		}
	}
}

void ASMPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASMPlayerCharacter, CurrentState);
	DOREPLIFETIME(ASMPlayerCharacter, bEnableCollision);
	DOREPLIFETIME(ASMPlayerCharacter, bCanControl);
}

void ASMPlayerCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	// 로컬 컨트롤러를 캐싱해야하기위한 조건입니다.
	if (IsLocallyControlled())
	{
		// 서버에선 OnRep_Controller가 호출되지 않고, 클라이언트에서는 PossessedBy가 호출되지 않기 때문에 서버는 여기서 컨트롤러를 캐싱합니다.
		StoredSMPlayerController = CastChecked<ASMPlayerController>(GetController());
	}
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
	CameraBoom->bEnableCameraLag = true;

	Camera->SetFieldOfView(CameraFOV);
}

void ASMPlayerCharacter::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputScalar = InputActionValue.Get<FVector2D>().GetSafeNormal();
	const FRotator CameraYawRotation(0.0, Camera->GetComponentRotation().Yaw, 0.0);
	const FVector ForwardDirection = FRotationMatrix(CameraYawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(CameraYawRotation).GetUnitAxis(EAxis::Y);
	const FVector MoveVector = (ForwardDirection * InputScalar.X) + (RightDirection * InputScalar.Y);

	const UCharacterMovementComponent* CachedCharacterMovement = GetCharacterMovement();
	if (CachedCharacterMovement->IsFalling())
	{
		const FVector NewMoveVector = MoveVector / 2.0f;
		AddMovementInput(NewMoveVector);
	}
	else
	{
		AddMovementInput(MoveVector);
	}
}

void ASMPlayerCharacter::InitCharacterControl()
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

void ASMPlayerCharacter::UpdateRotateToMousePointer()
{
	if (IsLocallyControlled())
	{
		const FVector MousePointingDirection = GetMousePointingDirection();
		if (MousePointingDirection == FVector::ZeroVector)
		{
			return;
		}

		const FRotator MousePointingRotation = FRotationMatrix::MakeFromX(MousePointingDirection).Rotator();
		const FRotator NewRotation = FRotator(0.0, MousePointingRotation.Yaw, 0.0);

		SetActorRotation(NewRotation);

		if (!HasAuthority())
		{
			ServerRotateToMousePointer(NewRotation.Yaw);
		}
	}
}

void ASMPlayerCharacter::ServerRotateToMousePointer_Implementation(float InYaw)
{
	// 트랜스폼은 리플리케이트 무브먼트 활성화 시 자동으로 모든 클라이언트에 동기화하기 때문에 서버에서만 처리해주면 됩니다.
	SetActorRotation(FRotator(0.0, InYaw, 0.0));
}

void ASMPlayerCharacter::OnRep_bCanControl()
{
	NET_LOG(LogSMNetwork, Log, TEXT("컨트롤 상태 변경 감지"))
	if (bCanControl)
	{
		EnableInput(StoredSMPlayerController);
	}
	else
	{
		DisableInput(StoredSMPlayerController);
	}
}

void ASMPlayerCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	UE_LOG(LogSMPlayerCharacter, Log, TEXT("Jumped!"));
}

void ASMPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	UE_LOG(LogSMPlayerCharacter, Log, TEXT("Landed! Floor Info: %s"), *Hit.GetComponent()->GetName());
}

float ASMPlayerCharacter::DistanceHeightFromFloor()
{
	FHitResult HitResult;
	const FVector Start = GetActorLocation() + (-GetActorUpVector() * 90.5f);
	const FVector End = GetActorLocation() + (-GetActorUpVector() * 10000.0f);
	FCollisionObjectQueryParams CollisionObjectQueryParams;
	FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(DistanceHeigh), false, this);
	CollisionObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	const bool bSuccess = GetWorld()->LineTraceSingleByObjectType(HitResult, Start, End, CollisionObjectQueryParams, CollisionQueryParams);
	if (bSuccess)
	{
		const float Distance = (HitResult.Location - Start).Size();
		UE_LOG(LogSMPlayerCharacter, Warning, TEXT("Height: %f"), Distance);

		return Distance;
	}
	else
	{
		return 0.0f;
	}
}

void ASMPlayerCharacter::Catch()
{
	HandleCatch();
}

void ASMPlayerCharacter::HandleCatch()
{
	if (IsLocallyControlled())
	{
		NET_LOG(LogSMNetwork, Log, TEXT("잡기 시전"));

		// 충돌 로직
		FHitResult HitResult;
		const FVector Start = GetActorLocation();
		const FVector End = Start + (GetActorForwardVector() * 300.0f);
		FCollisionObjectQueryParams CollisionObjectQueryParams;
		CollisionObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
		FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(Hold), false, this);
		const bool bSuccess = GetWorld()->SweepSingleByObjectType(HitResult, Start, End, FQuat::Identity, CollisionObjectQueryParams, FCollisionShape::MakeSphere(50.0f), CollisionQueryParams);

		// 충돌 시
		if (bSuccess)
		{
			NET_LOG(LogSMNetwork, Log, TEXT("잡기 적중"));
			ASMPlayerCharacter* HitPlayerCharacter = Cast<ASMPlayerCharacter>(HitResult.GetActor());
			if (HitPlayerCharacter)
			{
				ServerRPCPerformPull(HitPlayerCharacter);
			}
		}

		// 디버거
		const FVector Center = Start + (End - Start) * 0.5f;
		const FColor DrawColor = bSuccess ? FColor::Green : FColor::Red;
		DrawDebugCapsule(GetWorld(), Center, 150.0f, 50.0f, FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), DrawColor, false, 1.0f);
	}
}

void ASMPlayerCharacter::OnRep_CurrentState()
{
	switch (CurrentState)
	{
		case EPlayerCharacterState::Normal:
		{
			GetCharacterMovement()->GravityScale = 2.0f;
			break;
		}
		case EPlayerCharacterState::Caught:
		{
			GetCharacterMovement()->GravityScale = 0.0f;
			break;
		}
	}
}

void ASMPlayerCharacter::OnRep_bEnableCollision()
{
	SetActorEnableCollision(bEnableCollision);
}

void ASMPlayerCharacter::ServerRPCPerformPull_Implementation(ASMPlayerCharacter* InTargetCharacter)
{
	NET_LOG(LogSMNetwork, Log, TEXT("당기기 시작"));

	// 클라이언트 제어권 박탈 및 충돌 판정 비활성화
	InTargetCharacter->SetCanControl(false);
	InTargetCharacter->OnRep_bCanControl();

	InTargetCharacter->SetAutonomousProxy(false);

	InTargetCharacter->SetEnableCollision(false);
	InTargetCharacter->OnRep_bEnableCollision();

	InTargetCharacter->CurrentState = EPlayerCharacterState::Caught;
	InTargetCharacter->OnRep_CurrentState();

	// 당기기에 필요한 데이터 할당
	InTargetCharacter->PullData.bIsPulling = true;
	InTargetCharacter->PullData.ElapsedTime = 0.0f;
	InTargetCharacter->PullData.Caster = this;
	InTargetCharacter->PullData.StartLocation = InTargetCharacter->GetActorLocation();
}

void ASMPlayerCharacter::UpdatePerformPull(float DeltaSeconds)
{
	if (HasAuthority())
	{
		// 시전자로부터 자신을 향한 방향
		FVector CasterVector = GetActorLocation() - PullData.Caster->GetActorLocation();
		CasterVector.Z = 0.0;
		const FVector CasterDirection = CasterVector.GetSafeNormal();

		// 캡슐 반지름을 통해 EndLocation이 시전자의 위치와 겹치지 않도록 오프셋 지정
		const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
		PullData.EndLocation = PullData.Caster->GetActorLocation() + (CasterDirection * CapsuleRadius * 2);

		// 선형 보간
		PullData.ElapsedTime += DeltaSeconds;
		const float Alpha = FMath::Clamp(PullData.ElapsedTime / PullData.TotalTime, 0.0f, 1.0f);
		const FVector NewLocation = FMath::Lerp(PullData.StartLocation, PullData.EndLocation, Alpha);
		SetActorLocation(NewLocation);

		// 디버거
		DrawDebugLine(GetWorld(), PullData.StartLocation, PullData.EndLocation, FColor::Cyan, false, 0.1f);

		if (Alpha >= 1.0f)
		{
			HandlePullEnd();
		}
	}
}

void ASMPlayerCharacter::HandlePullEnd()
{
	if (HasAuthority())
	{
		NET_LOG(LogSMNetwork, Log, TEXT("당기기 종료"))

		PullData.bIsPulling = false;
		MulticastRPCAttachToCaster(PullData.Caster, this);
		StoredSMPlayerController->SetViewTargetWithBlend(PullData.Caster, 0.1f);
	}
}

void ASMPlayerCharacter::MulticastRPCAttachToCaster_Implementation(AActor* InCaster, AActor* InTarget)
{
	NET_LOG(LogSMNetwork, Log, TEXT("어태치 시작"))

	const FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
	const ASMPlayerCharacter* SMPlayerCharacter = Cast<ASMPlayerCharacter>(InCaster);
	if (SMPlayerCharacter)
	{
		InTarget->AttachToComponent(SMPlayerCharacter->GetMesh(), AttachmentTransformRules, TEXT("HoldSocket"));
	}
}
