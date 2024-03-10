// Fill out your copyright notice in the Description page of Project Settings.


#include "SMPlayerCharacter.h"

#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SMCharacterAssetData.h"
#include "Animation/SMCharacterAnimInstance.h"
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
	bUseControllerRotationRoll = true;
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;

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
	bEnableMovement = true;
	bCanControl = true;

	// Design
	CatchTime = 0.25f;
	StandUpTime = 3.0f;
}

void ASMPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	StoredSMAnimInstance = CastChecked<USMCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	StoredSMAnimInstance->OnSmashEnded.BindUObject(this, &ASMPlayerCharacter::SmashEnded);
	StoredSMAnimInstance->OnStandUpEnded.BindUObject(this, &ASMPlayerCharacter::StandUpEnded);
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
		AimPlane->AttachToActor(this, AttachmentTransformRules);
	}

	InitCharacterControl();

	InitDesignData();
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
		EnhancedInputComponent->BindAction(AssetData->SmashAction, ETriggerEvent::Started, this, &ASMPlayerCharacter::Smash);
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
	DOREPLIFETIME(ASMPlayerCharacter, bEnableMovement);
	DOREPLIFETIME(ASMPlayerCharacter, CaughtCharacter);
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

void ASMPlayerCharacter::InitDesignData()
{
	PullData.TotalTime = CatchTime;
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
	FHitResult HitResult;
	const bool Succeed = StoredSMPlayerController->GetHitResultUnderCursor(TC_AIM_PLANE, false, HitResult);
	if (Succeed)
	{
		const FVector MouseLocation = HitResult.Location;
		const FVector MouseDirection = (MouseLocation - GetActorLocation()).GetSafeNormal();
		return MouseDirection;
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

		StoredSMPlayerController->SetControlRotation(NewRotation);
	}
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

void ASMPlayerCharacter::SetEnableMovement(bool bInEnableMovement)
{
	if (HasAuthority())
	{
		bEnableMovement = bInEnableMovement;
		OnRep_bEnableMovement();
	}
}

void ASMPlayerCharacter::SetCurrentState(EPlayerCharacterState InState)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentState = InState;
	OnRep_CurrentState();
}

void ASMPlayerCharacter::SetEnableCollision(bool bInEnableCollision)
{
	if (!HasAuthority())
	{
		return;
	}

	bEnableCollision = bInEnableCollision;
	OnRep_bEnableCollision();
}

void ASMPlayerCharacter::SetCanControl(bool bInEnableControl)
{
	if (!HasAuthority())
	{
		return;
	}

	bCanControl = bInEnableControl;
	OnRep_bCanControl();
}

void ASMPlayerCharacter::OnRep_bCanControl()
{
	if (bCanControl)
	{
		NET_LOG(LogSMCharacter, Log, TEXT("컨트롤 활성화"))
		EnableInput(StoredSMPlayerController);
		bUseControllerRotationYaw = true;
	}
	else
	{
		NET_LOG(LogSMCharacter, Log, TEXT("컨트롤 비활성화"))
		DisableInput(StoredSMPlayerController);
		bUseControllerRotationYaw = false;
	}
}

void ASMPlayerCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	NET_LOG(LogSMCharacter, Log, TEXT("Jumped!"));
}

void ASMPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	NET_LOG(LogSMCharacter, Log, TEXT("Landed! Floor Info: %s"), *Hit.GetComponent()->GetName());
}

TArray<ASMPlayerCharacter*> ASMPlayerCharacter::GetCharactersExcludingServerAndCaster()
{
	TArray<ASMPlayerCharacter*> Result;
	for (const APlayerController* PlayerController : TActorRange<APlayerController>(GetWorld()))
	{
		if (!PlayerController || PlayerController->IsLocalController() || PlayerController == GetController())
		{
			continue;
		}

		ASMPlayerCharacter* SMPlayerCharacter = Cast<ASMPlayerCharacter>(PlayerController->GetPawn());
		if (SMPlayerCharacter)
		{
			Result.Add(SMPlayerCharacter);
		}
	}

	return Result;
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
		NET_LOG(LogSMCharacter, Warning, TEXT("Height: %f"), Distance);

		return Distance;
	}
	else
	{
		return 0.0f;
	}
}

void ASMPlayerCharacter::SetCaughtCharacter(ASMPlayerCharacter* InCaughtCharacter)
{
	if (HasAuthority())
	{
		CaughtCharacter = InCaughtCharacter;
	}
}

void ASMPlayerCharacter::Catch()
{
	if (!CaughtCharacter)
	{
		StoredSMAnimInstance->PlayCatch();
		ServerRPCPlayCatchAnimation();
	}
}

void ASMPlayerCharacter::HandleCatch()
{
	if (IsLocallyControlled())
	{
		NET_LOG(LogSMCharacter, Log, TEXT("잡기 시전"));

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
			NET_LOG(LogSMCharacter, Log, TEXT("잡기 적중"));
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
			NET_LOG(LogSMCharacter, Log, TEXT("현재 캐릭터 상태: Normal"));
			break;
		}
		case EPlayerCharacterState::Caught:
		{
			NET_LOG(LogSMCharacter, Log, TEXT("현재 캐릭터 상태: Caught"));
			break;
		}
		case EPlayerCharacterState::Down:
		{
			NET_LOG(LogSMCharacter, Log, TEXT("현재 캐릭터 상태: Down"));
			break;
		}
	}
}

void ASMPlayerCharacter::OnRep_bEnableMovement()
{
	if (bEnableMovement)
	{
		NET_LOG(LogSMCharacter, Log, TEXT("움직임 활성화"));
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	else
	{
		NET_LOG(LogSMCharacter, Log, TEXT("움직임 비활성화"));
		GetCharacterMovement()->SetMovementMode(MOVE_None);
	}
}

void ASMPlayerCharacter::OnRep_bEnableCollision()
{
	NET_LOG(LogSMCharacter, Log, TEXT("충돌 %s"), bEnableCollision ? TEXT("활성화") : TEXT("비활성화"));
	SetActorEnableCollision(bEnableCollision);
}

void ASMPlayerCharacter::ServerRPCPerformPull_Implementation(ASMPlayerCharacter* InTargetCharacter)
{
	NET_LOG(LogSMCharacter, Log, TEXT("당기기 시작"));

	// 클라이언트 제어권 박탈 및 충돌 판정 비활성화
	InTargetCharacter->SetAutonomousProxy(false);
	InTargetCharacter->SetCanControl(false);
	InTargetCharacter->SetEnableMovement(false);
	InTargetCharacter->SetEnableCollision(false);

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
		NET_LOG(LogSMCharacter, Log, TEXT("당기기 종료"))
		PullData.bIsPulling = false;

		NET_LOG(LogSMCharacter, Log, TEXT("어태치 시작"))
		MulticastRPCAttachToCaster(PullData.Caster, this);
		StoredSMPlayerController->SetViewTargetWithBlend(PullData.Caster, 0.1f);
	}
}

void ASMPlayerCharacter::MulticastRPCAttachToCaster_Implementation(ASMPlayerCharacter* InCaster, ASMPlayerCharacter* InTarget)
{
	const FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
	if (InCaster && InTarget)
	{
		// 플레이어의 상태를 잡힘으로 변경합니다.
		if (HasAuthority())
		{
			InCaster->SetCaughtCharacter(this);
		}

		InTarget->SetCurrentState(EPlayerCharacterState::Caught);
		InTarget->AttachToComponent(InCaster->GetMesh(), AttachmentTransformRules, TEXT("CatchSocket"));
		if (!HasAuthority())
		{
			InTarget->StoredSMAnimInstance->PlayCaught();
		}
	}
}

void ASMPlayerCharacter::ServerRPCPlayCatchAnimation_Implementation() const
{
	for (const APlayerController* PlayerController : TActorRange<APlayerController>(GetWorld()))
	{
		if (!PlayerController->IsLocalController())
		{
			if (PlayerController != GetController())
			{
				const ASMPlayerCharacter* SMPlayerCharacter = Cast<ASMPlayerCharacter>(PlayerController->GetPawn());
				SMPlayerCharacter->ClientRPCPlayCatchAnimation(this);
			}
		}
	}
}

void ASMPlayerCharacter::ClientRPCPlayCatchAnimation_Implementation(const ASMPlayerCharacter* InPlayAnimationCharacter) const
{
	InPlayAnimationCharacter->StoredSMAnimInstance->PlayCatch();
}

void ASMPlayerCharacter::Smash()
{
	NET_LOG(LogSMCharacter, Warning, TEXT("매치기 트리거"));

	if (CaughtCharacter)
	{
		StoredSMAnimInstance->PlaySmash();
		ServerRPCPlaySmashAnimation();
	}
}

void ASMPlayerCharacter::ServerRPCPlaySmashAnimation_Implementation()
{
	NET_LOG(LogSMCharacter, Log, TEXT("매치기 시작"));

	SetCanControl(false);

	for (const ASMPlayerCharacter* CharacterToAnimation : GetCharactersExcludingServerAndCaster())
	{
		if (CharacterToAnimation)
		{
			CharacterToAnimation->ClientRPCPlaySmashAnimation(this);
		}
	}
}

void ASMPlayerCharacter::ClientRPCPlaySmashAnimation_Implementation(const ASMPlayerCharacter* InPlayAnimationCharacter) const
{
	if (InPlayAnimationCharacter)
	{
		InPlayAnimationCharacter->StoredSMAnimInstance->PlaySmash();
	}
}

void ASMPlayerCharacter::HandleSmash()
{
	// 캐스터 클라이언트에서만 실행되야합니다.
	if (IsLocallyControlled())
	{
		NET_LOG(LogSMCharacter, Log, TEXT("매치기 처리"));

		if (ensureMsgf(CaughtCharacter, TEXT("잡힌 캐릭터가 존재하지 않습니다.")))
		{
			CaughtCharacter->StoredSMAnimInstance->PlayKnockDown();

			// 클라이언트에서 먼저 매쳐진 위치로 대상을 이동시킵니다. 이후 서버에도 해당 데이터를 보내 동기화시킵니다.
			// 매쳐지는 위치를 직접 정하는 이유는 애니메이션에만 의존해서 의도된 위치에 매쳐지도록 조정하기 어렵기 때문입니다.
			const FVector DownLocation = GetActorLocation() + (GetActorForwardVector() * 150.0f);
			FRotator DownRotation = FRotationMatrix::MakeFromX(GetActorForwardVector()).Rotator();
			DownRotation.Pitch = 0.0;
			DownRotation.Roll = 0.0;
			// CaughtCharacter->SetActorLocationAndRotation(DownLocation, DownRotation);
			ServerRPCDetachToCaster(DownLocation, DownRotation);
		}
	}
}

void ASMPlayerCharacter::SmashEnded(UAnimMontage* PlayAnimMontage, bool bInterrupted)
{
	if (IsLocallyControlled())
	{
		ServerRPCSmashEnded();
	}
}

void ASMPlayerCharacter::ServerRPCDetachToCaster_Implementation(FVector InLocation, FRotator InRotation)
{
	if (!HasAuthority())
	{
		return;
	}

	// TODO: 캐스터의 입력 활성화: 매치기 애니메이션 종료 후
	// 이는 매치기 종료 애니메이션에 델리게이트를 바인드 한 후 이 시점에 입력을 활성화하도록 구성하면 된다.
	// 일단락: 매치기 애니메이션 종료 후 캐스터의 입력 활성화되도록 로직 구성

	// TODO: 매치기 당한 캐릭터가 매치기 종료 후 스스로 제어권을 확보할 수 있도록 로직 구성
	// 서버에서 해당 캐릭터의 기상 애니메이션을 재생하고 이 애니메이션이 끝나면 제어권을 확보한다.
	// 1. 서버에서 해당 캐릭터의 기상 애니메이션을 멀티캐스트로 재생한다.
	// 2. 이 캐릭터를 조종하고 있는 클라이언트 측에서는 이 애니메이션 재생이 종료될때 제어권을 확보해야하는데 이를 위해 기상기가 종료되는 시점의 델리게이트에 제어권을 되찾는 함수를 바인딩해둔다. 물론 다른 캐릭터들도 기상을 할때가 있기 때문에 일괄적으로 모두 바인드 후 제어권을 찾는 함수를 바인드해둔다. 그리고 이 함수는 로컬에서만 실행되도록하고 서버 RPC를 사용해 본인의 제어권을 되찾도록 설계한다. 이때 동기화되 될 것이다.
	// 3. 바인드는 캐릭터의 포스트 이니셜라이즈드 컴포넌트에서 진행한다.
	// 4. 애님 인스턴스에서는 기상 애니메이션의 종료 시점에 연결된 델리게이트를 public 변수로 갖고 있어야한다. 이 델리게이트에 캐릭터의 함수를 연결시킬것이다.
	// 문제: 타이머에는 인자 사용이 불가능해 타이머 이후 서버에서 해당 캐릭터를 기상시킬 수 없다. 만약 멤버 변수로 할당해 사용하려 한다면 매치고 5초 안에 다른 대상을 매치려고 하는경우 포인터가 달라지게되 문제가 생긴다.
	// 위와 같은 문제는 넘어져있는 애니메이션과 기상하는 애니메이션이 분리되어 있어 따로 트리거 해줘야하기 때문에 생기는 문제다.
	// 이렇게 설계한 이유는 넘어져 있는 시간은 변동될 수 있기 때문이다.
	// 그럼 어떻게 해당 캐릭터에게 인자를 사용하지 않고 기상 애니메이션을 재생시킬까?
	// 1. 넘어져 있는 애니메이션을 매쳐지고 난 뒤 5초만 재생시키고 바로 기상으로 이어지도록 애니메이션을 이은다.
	// 2. 애시당초 매쳐지는 순간 타이머에 this가 아닌 매쳐지는 캐릭터의 포인터를 넘겨 스스로 실행되도록한다.
	// 해결
	// 타이머에 포인터를 매쳐지는 캐릭터로 넘겨 서버측에서 매쳐지는 캐릭터를 기준으로 로직이 전개된다.
	// 애니메이션 종료후 델리게이트를 호출하는데 이 델리게이트는 로컬에서만 처리되도록 설계해 해당 캐릭터의 제어권을 되찾으면 된다.
	// 일단락: 스스로 일어나며 제어권을 회복하도록 로직 구성 완료

	// TODO: 매치기 당한 캐릭터의 위치 동기화
	// 오토너머스 프록시로 재설정해준 뒤에 NetMulticastRPC를 통한 SetActorLocation으로 위치를 재설정해준다. 약간의 튐이 발생할 수는 있다.
	// 회전의 경우 SetControlRotation을 통해 설정해주어 알아서 보간되도록 한다.
	// 기상 애니메이션 시작시 미리 위치를 동기화해두는 것을 목표로 작성한다.
	// 이 때 활성화해야할 목록 콜리전, 오토너머스, 무브먼트. 추가로 카메라도 복구해야한다.
	// 카메라는 디태치 되는 타이밍에 하도록 처리한다.
	// 기상 애니메이션 종료시에는 입력을 활성화 시켜준다.
	// 일단 락

	// TODO: 애니메이션 동기화 오류, 스매시 애니메이션 중단 오류
	// 로직 이상으로 캐스터 클라이언트에서 1번 더 실행했고, 이로 인해 애니메이션이 중단되었던 것.
	// 일단 락

	// TODO: 논캐스터 클라이언트에 넘어진 애니메이션 재생 오류
	// 호출 자체를 했음
	// 일단 락

	// TODO: 

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, CaughtCharacter.Get(), &ASMPlayerCharacter::MulticastRPCPlayStandUpAnimation, StandUpTime);

	ASMPlayerCharacter* CachedCaughtCharacter = CaughtCharacter;
	MulticastRPCDetachToCaster(this, CaughtCharacter);
	SetCaughtCharacter(nullptr);

	CachedCaughtCharacter->SetAutonomousProxy(true);
	CachedCaughtCharacter->SetEnableCollision(true);
	CachedCaughtCharacter->SetEnableMovement(true);
}

void ASMPlayerCharacter::ServerRPCSmashEnded_Implementation()
{
	if (HasAuthority())
	{
		SetCanControl(true);
	}
}

void ASMPlayerCharacter::MulticastRPCPlayStandUpAnimation_Implementation()
{
	if (!HasAuthority())
	{
		StoredSMAnimInstance->PlayStandUp();
	}
}

void ASMPlayerCharacter::StandUpEnded(UAnimMontage* PlayAnimMontage, bool bInterrupted)
{
	if (IsLocallyControlled())
	{
		NET_LOG(LogSMCharacter, Log, TEXT("기상 애니메이션 종료"));
		ServerRPCStandUpEnded();
	}
}

void ASMPlayerCharacter::ServerRPCStandUpEnded_Implementation()
{
	SetCurrentState(EPlayerCharacterState::Normal);
	SetCanControl(true);
}

void ASMPlayerCharacter::MulticastRPCDetachToCaster_Implementation(ASMPlayerCharacter* InCaster, ASMPlayerCharacter* InTarget)
{
	if (InCaster && InTarget)
	{
		NET_LOG(LogSMCharacter, Log, TEXT("디태치"));
		const FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, false);
		InTarget->DetachFromActor(DetachmentTransformRules);

		if (HasAuthority())
		{
			// 뷰타겟 설정은 서버에서만 실행해주면 알아서 동기화됩니다.
			InTarget->StoredSMPlayerController->SetViewTargetWithBlend(InTarget, 0.3f);
		}

		if (!HasAuthority())
		{
			InTarget->StoredSMAnimInstance->PlayKnockDown();
		}
	}
}
