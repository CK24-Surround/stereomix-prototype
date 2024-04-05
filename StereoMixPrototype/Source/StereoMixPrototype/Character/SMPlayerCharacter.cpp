// Fill out your copyright notice in the Description page of Project Settings.


#include "SMPlayerCharacter.h"

#include "Data/SMTeam.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "SMCharacterAssetData.h"
#include "SMSmashComponent.h"
#include "SMTeamComponent.h"
#include "Ability/SMCharacterAbilityManagerComponent.h"
#include "Ability/SMGrabSmashAbilityComponent.h"
#include "Animation/SMCharacterAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "CharacterStat/SMCharacterStatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Design/SMPlayerCharacterDesignData.h"
#include "Game/SMGameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Log/SMLog.h"
#include "Net/UnrealNetwork.h"
#include "Physics/SMCollision.h"
#include "Player/AimPlane.h"
#include "Player/SMPlayerController.h"
#include "Player/SMPlayerState.h"
#include "Projectile/SMRangedAttackProjectile.h"
#include "UI/SMPostureGaugeWidget.h"

ASMPlayerCharacter::ASMPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetMesh()->SetCollisionProfileName("NoCollision");

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = 700.0f;
	Movement->MaxAcceleration = 10000.0f;
	Movement->BrakingDecelerationWalking = 1000.0f;
	Movement->BrakingDecelerationFalling = 0.0f; // ~ 100 중 선택
	Movement->AirControl = 1.0f;
	Movement->GravityScale = 2.0f;
	Movement->JumpZVelocity = 700.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom);

	PostureGauge = CreateDefaultSubobject<UWidgetComponent>(TEXT("PostureGauge"));
	PostureGauge->SetupAttachment(GetRootComponent());
	PostureGauge->SetWidgetSpace(EWidgetSpace::Screen);
	PostureGauge->SetDrawAtDesiredSize(true);
	if (AssetData->PostureGauge)
	{
		PostureGauge->SetWidgetClass(AssetData->PostureGauge);
	}

	TeamComponent = CreateDefaultSubobject<USMTeamComponent>(TEXT("Team"));
	SmashComponent = CreateDefaultSubobject<USMSmashComponent>(TEXT("Smash"));
	HitBoxComponent = CreateDefaultSubobject<USphereComponent>(TEXT("HitBox"));
	HitBoxComponent->SetupAttachment(GetRootComponent());
	HitBoxComponent->SetCollisionProfileName(CP_PLAYER_HIT_BOX);
	HitBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	MoveTrailEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MoveTrailEffect"));
	MoveTrailEffectComponent->SetupAttachment(GetMesh());
	MoveTrailEffectComponent->SetRelativeLocation(FVector(0.0, 0.0, 80.0));
	MoveTrailEffectComponent->SetAsset(AssetData->MoveTrailEffect);

	InitCamera();

	CurrentState = EPlayerCharacterState::Normal;

	// Add ability
	GetAbilityManager()->AddAbility<USMGrabSmashAbilityComponent>(TEXT("GrabSmash"));
}

void ASMPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// Server에서만 호출됩니다.
void ASMPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	SMPlayerController = CastChecked<ASMPlayerController>(NewController);
}

void ASMPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	EnhancedInputComponent->BindAction(AssetData->MoveAction, ETriggerEvent::Triggered, this,
	                                   &ASMPlayerCharacter::Move);

	EnhancedInputComponent->BindAction(AssetData->FutureBaseTeamSelectAction, ETriggerEvent::Started, this,
	                                   &ASMPlayerCharacter::ServerRPCFutureBassTeamSelect);
	EnhancedInputComponent->BindAction(AssetData->RockTeamSelectAction, ETriggerEvent::Started, this,
	                                   &ASMPlayerCharacter::ServerRPCRockTeamSelect);

	// Set ability input
	GetAbilityManager()->GetAbility(TEXT("GrabSmash"))->BindInput(EnhancedInputComponent, AssetData->SmashAction);
}

void ASMPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASMPlayerCharacter, CurrentState);
}

void ASMPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();


	SMPlayerState = GetPlayerState<ASMPlayerState>();

	// 클라이언트에서 로컬 캐릭터만 컨트롤러를 가지고 있음
	if (APlayerController* PC = SMPlayerState->GetPlayerController())
	{
		SMPlayerController = CastChecked<ASMPlayerController>(PC);
	}
}

void ASMPlayerCharacter::OnRep_CurrentState(const EPlayerCharacterState& OldState) const
{
	NET_LOG(LogSMCharacter, Log, TEXT("Change State: %s->%s"),
	        *UEnum::GetValueAsString(OldState),
	        *UEnum::GetValueAsString(CurrentState));
}

void ASMPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		AimPlane = GetWorld()->SpawnActor<AAimPlane>();
		const FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
		AimPlane->AttachToActor(this, AttachmentTransformRules);

		InitializeInputSystem();
	}

	// 서버는 UI 업데이트가 필요하지 않습니다.
	if (!HasAuthority())
	{
		PostureGaugeWidget = Cast<USMPostureGaugeWidget>(PostureGauge->GetWidget());
		if (PostureGaugeWidget)
		{
			Stat->OnChangedPostureGauge.AddDynamic(PostureGaugeWidget, &USMPostureGaugeWidget::UpdatePostureGauge);
			PostureGaugeWidget->UpdatePostureGauge(Stat->GetCurrentPostureGauge(), Stat->GetBaseStat().MaxPostureGauge);
		}
	}
}

void ASMPlayerCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsLocallyControlled() && InputEnabled())
	{
		UpdateRotateToMousePointer();

		FVector MoveDirection = GetCharacterMovement()->Velocity.GetSafeNormal();
		MoveDirection.Z = 0.0f;
		const FRotator MoveDirectionRotation = FRotationMatrix::MakeFromX(MoveDirection).Rotator();
		MoveTrailEffectComponent->SetWorldRotation(MoveDirectionRotation);
	}
}

void ASMPlayerCharacter::InitCamera() const
{
	const FRotator CameraRotation(-45.0f, 0.0, 0.0);
	constexpr float CameraDistance = 750.0f;
	constexpr float CameraFOV = 90.0f;

	CameraBoom->SetRelativeRotation(CameraRotation);
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->TargetArmLength = CameraDistance;
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = true;

	Camera->SetFieldOfView(CameraFOV);
}

void ASMPlayerCharacter::InitializeInputSystem() const
{
	if (!IsLocallyControlled())
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(AssetData->DefaultMappingContext, 0);
	}
	FInputModeGameOnly InputModeGameOnly;
	InputModeGameOnly.SetConsumeCaptureMouseDown(false);
	PlayerController->SetInputMode(InputModeGameOnly);
}

void ASMPlayerCharacter::UpdateRotateToMousePointer()
{
	const FVector MousePointingDirection = SMPlayerController->GetMousePointingDirection();
	if (MousePointingDirection == FVector::ZeroVector)
	{
		return;
	}

	const FRotator MousePointingRotation = FRotationMatrix::MakeFromX(MousePointingDirection).Rotator();
	const FRotator NewRotation = FRotator(0.0, MousePointingRotation.Yaw, 0.0);
	AddControllerYawInput(MousePointingRotation.Yaw);
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

void ASMPlayerCharacter::SetCurrentState(const EPlayerCharacterState InState)
{
	if (!HasAuthority())
	{
		return;
	}

	const EPlayerCharacterState OldState = CurrentState;
	CurrentState = InState;
	OnRep_CurrentState(OldState);
}

/*
void ASMPlayerCharacter::ApplyStunned()
{
	NET_LOG(LogSMCharacter, Log, TEXT("기절 상태 적용"));
	SetStunned(true);
	SetCurrentState(EPlayerCharacterState::Stun);

	StoredSMAnimInstance->PlayStun();
	MulticastRPCPlayStunVisualEffect();
	const float StunEndTime = DesignData->StunTime - StoredSMAnimInstance->GetStunEndLength();
	GetWorldTimerManager().SetTimer(StunTimerHandle, this, &ASMPlayerCharacter::RecoverStunned, StunEndTime, false);
}

void ASMPlayerCharacter::MulticastRPCPlayStunVisualEffect_Implementation()
{
	if (!HasAuthority())
	{
		StoredSMAnimInstance->PlayStun();

		StunEffectComponent->SetAsset(AssetData->StunLoopEffect);
		StunEffectComponent->ActivateSystem();
	}
}

void ASMPlayerCharacter::RecoverStunned()
{
	// 만약 기절 상태가 아니면(잡혀있다면) 아래 코드를 수행하지 않아야 합니다. 
	if (CurrentState == EPlayerCharacterState::Stun)
	{
		StoredSMAnimInstance->PlayStunEnd();
		MulticastRPCPlayStunEndAnimation();
	}
}

void ASMPlayerCharacter::MulticastRPCPlayStunEndAnimation_Implementation()
{
	if (!HasAuthority())
	{
		StoredSMAnimInstance->PlayStunEnd();
	}
}

void ASMPlayerCharacter::StunEnded(UAnimMontage* InAnimMontage, bool bInterrupted)
{
	if (bInterrupted || GetCurrentState() == EPlayerCharacterState::Caught)
	{
		NET_LOG(LogSMCharacter, Log, TEXT("기절 애니메이션 중단"));
		return;
	}

	Stat->ClearPostureGauge();
	SetStunned(false);
	SetCurrentState(EPlayerCharacterState::Normal);

	MulticastRPCPlayStunEndedVisualEffect();
}

void ASMPlayerCharacter::MulticastRPCPlayStunEndedVisualEffect_Implementation()
{
	if (!HasAuthority())
	{
		StunEffectComponent->SetAsset(AssetData->StunEndEffect);
		StunEffectComponent->ActivateSystem();
	}
}

void ASMPlayerCharacter::OnRep_bIsStunned()
{
	NET_LOG(LogSMCharacter, Log, TEXT("스턴 상태: %s"), bIsStunned ? TEXT("활성화") : TEXT("비활성화"));
	SetCanControl(!bIsStunned);
	bIsStunned ? SetCollisionProfileName(CP_STUNNED) : SetCollisionProfileName(CP_PLAYER_HIT_BOX);
}
*/

TArray<ASMPlayerCharacter*> ASMPlayerCharacter::GetCharactersExcludingServerAndCaster() const
{
	TArray<ASMPlayerCharacter*> Result;
	for (const APlayerController* PlayerController : TActorRange<APlayerController>(GetWorld()))
	{
		if (!PlayerController || PlayerController->IsLocalController() || PlayerController == GetController())
		{
			continue;
		}
		if (ASMPlayerCharacter* SMPlayerCharacter = Cast<ASMPlayerCharacter>(PlayerController->GetPawn()))
		{
			Result.Add(SMPlayerCharacter);
		}
	}

	return Result;
}

float ASMPlayerCharacter::DistanceHeightFromFloor() const
{
	FHitResult HitResult;
	const FVector Start = GetActorLocation() + (-GetActorUpVector() * 90.5f);
	const FVector End = GetActorLocation() + (-GetActorUpVector() * 10000.0f);
	FCollisionObjectQueryParams CollisionObjectQueryParams;
	FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(DistanceHeigh), false, this);
	CollisionObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	if (GetWorld()->LineTraceSingleByObjectType(HitResult, Start, End,
	                                            CollisionObjectQueryParams,
	                                            CollisionQueryParams))
	{
		const float Distance = (HitResult.Location - Start).Size();
		NET_LOG(LogSMCharacter, Log, TEXT("Height: %f"), Distance);

		return Distance;
	}
	return 0.0f;
}

/*

void ASMPlayerCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
}

void ASMPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	NET_LOG(LogSMCharacter, Log, TEXT("Landed! Floor Info: %s"), *Hit.GetComponent()->GetName());
}


void ASMPlayerCharacter::Catch()
{
	if (bCanCatch)
	{
		NET_LOG(LogSMCharacter, Log, TEXT("잡기 시전"));
		bCanCatch = false;

		CatchLocation = GetMouseCursorLocation();

		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ASMPlayerCharacter::CanCatch,
		                                DesignData->CatchCoolDownTime);

		if (!CaughtCharacter)
		{
			StoredSMAnimInstance->PlayCatch();
			ServerRPCPlayCatchAnimation();
		}
		GetCharacterMovement()->AddForce(FVector(0.f, 100.0f, 0.f));
	}
}

void ASMPlayerCharacter::CanCatch()
{
	bCanCatch = true;
}

void ASMPlayerCharacter::ServerRPCPlayCatchAnimation_Implementation()
{
	for (const ASMPlayerCharacter* RemoteCharacter : GetCharactersExcludingServerAndCaster())
	{
		RemoteCharacter->ClientRPCPlayCatchAnimation(this);
	}
}

void ASMPlayerCharacter::ClientRPCPlayCatchAnimation_Implementation(
	const ASMPlayerCharacter* InPlayAnimationCharacter) const
{
	if (InPlayAnimationCharacter)
	{
		InPlayAnimationCharacter->StoredSMAnimInstance->PlayCatch();
	}
}

void ASMPlayerCharacter::AnimNotify_Catch()
{
	if (IsLocallyControlled())
	{
		NET_LOG(LogSMCharacter, Log, TEXT("잡기 시작"));

		// 나중에 기절 종료 애니메이션이 재생되는 중에 잡기가 성사되고, 서버지연으로인해 잡기가 먼저 처리 된 뒤 기절 종료 애니가 재생 마무리 될경우 버그를 방지하기 위해 서버측에서 유효하지 않은 상황이라면 롤백하는 검증 코드가 필요합니다.

		// 충돌 로직
		TArray<FOverlapResult> HitResults;
		const FVector Start = GetActorLocation();
		FCollisionObjectQueryParams CollisionObjectQueryParams;
		CollisionObjectQueryParams.AddObjectTypesToQuery(OC_STUNNED);
		FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(Catch), false, this);
		bool bSuccess = GetWorld()->OverlapMultiByObjectType(HitResults, Start, FQuat::Identity,
		                                                     CollisionObjectQueryParams,
		                                                     FCollisionShape::MakeSphere(DesignData->CatchMaxDistance),
		                                                     CollisionQueryParams);

		// 충돌 시
		if (bSuccess)
		{
			bSuccess = false;
			TArray<ASMPlayerCharacter*> CanCatchCharacters;
			for (const auto& HitResult : HitResults)
			{
				ASMPlayerCharacter* HitPlayerCharacter = Cast<ASMPlayerCharacter>(HitResult.GetActor());
				if (!HitPlayerCharacter || HitPlayerCharacter->GetCaughtCharacter())
				{
					continue;
				}

				if (DesignData->bCanTeamCatch || TeamComponent->GetCurrentTeam() != HitPlayerCharacter->TeamComponent->
					GetCurrentTeam())
				{
					NET_LOG(LogSMCharacter, Log, TEXT("잡기 적중"));
					CanCatchCharacters.Add(HitPlayerCharacter);

					bSuccess = true;
				}
			}

			if (bSuccess)
			{
				ASMPlayerCharacter* ClosestCharacterToMouse = nullptr;
				float ClosestCharacterToMouseDistance = FLT_MAX;
				for (const auto& CanCatchCharacter : CanCatchCharacters)
				{
					const float Distance = FVector::DistSquared(CatchLocation, CanCatchCharacter->GetActorLocation());
					if (ClosestCharacterToMouseDistance > Distance)
					{
						ClosestCharacterToMouseDistance = Distance;
						ClosestCharacterToMouse = CanCatchCharacter;
					}
				}

				ServerRPCPerformPull(ClosestCharacterToMouse);

				ClosestCharacterToMouse->StunEffectComponent->DeactivateImmediate();
			}
		}

		// 디버거
		const FVector Center = Start;
		const FColor DrawColor = bSuccess ? FColor::Green : FColor::Red;
		DrawDebugSphere(GetWorld(), Center, DesignData->CatchMaxDistance, 16, DrawColor, false, 1.0f);
	}
}

void ASMPlayerCharacter::ServerRPCPerformPull_Implementation(ASMPlayerCharacter* InTargetCharacter)
{
	NET_LOG(LogSMCharacter, Log, TEXT("당기기 시작"));

	// 기절 타이머 초기화
	// 현재는 잡힌 도중 기절에서 탈출할 수 없도록 설계했습니다. 추후 탈출 로직 추가가 필요합니다.
	// InTargetCharacter->GetWorldTimerManager().ClearTimer(InTargetCharacter->StunTimerHandle);

	// 클라이언트 제어권 박탈 및 충돌 판정 비활성화
	InTargetCharacter->SetCanControl(false);
	InTargetCharacter->SetEnableMovement(false);
	InTargetCharacter->SetEnableCollision(false);

	InTargetCharacter->SetCurrentState(EPlayerCharacterState::Caught);

	// 당기기에 필요한 데이터 할당
	InTargetCharacter->PullData.bIsPulling = true;
	InTargetCharacter->PullData.ElapsedTime = 0.0f;
	InTargetCharacter->PullData.Caster = this;
	InTargetCharacter->PullData.StartLocation = InTargetCharacter->GetActorLocation();
	InTargetCharacter->ClientRPCLastTimeCheck();

	// 클라이언트 RPC
	for (const ASMPlayerCharacter* NeedPlayingClient : GetCharactersExcludingServerAndCaster())
	{
		if (NeedPlayingClient)
		{
			NeedPlayingClient->ClientRPCPlayCaughtVisualEffect(InTargetCharacter);
		}
	}
}

void ASMPlayerCharacter::ClientRPCPlayCaughtVisualEffect_Implementation(ASMPlayerCharacter* NeedPlayingClient) const
{
	if (NeedPlayingClient)
	{
		NeedPlayingClient->StunEffectComponent->DeactivateImmediate();
	}
}

void ASMPlayerCharacter::ClientRPCLastTimeCheck_Implementation()
{
	PullData.LastServerTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
}

void ASMPlayerCharacter::UpdatePerformPull()
{
	if (!HasAuthority())
	{
		return;
	}

	// 시전자로부터 자신을 향한 방향
	FVector CasterVector = GetActorLocation() - PullData.Caster->GetActorLocation();
	CasterVector.Z = 0.0;
	const FVector CasterDirection = CasterVector.GetSafeNormal();

	// 캡슐 반지름을 통해 EndLocation이 시전자의 위치와 겹치지 않도록 오프셋 지정
	const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	PullData.EndLocation = PullData.Caster->GetActorLocation() + (CasterDirection * CapsuleRadius * 2);

	// 선형 보간
	PullData.ElapsedTime += GetWorld()->GetDeltaSeconds();
	const float Alpha = FMath::Clamp(PullData.ElapsedTime / PullData.TotalTime, 0.0f, 1.0f);
	const FVector NewLocation = FMath::Lerp(PullData.StartLocation, PullData.EndLocation, Alpha);
	SetActorLocation(NewLocation);
	ClientRPCInterpolationPull(NewLocation);

	// 디버거
	DrawDebugLine(GetWorld(), PullData.StartLocation, PullData.EndLocation, FColor::Cyan, false, 0.1f);

	if (Alpha >= 1.0f)
	{
		HandlePullEnd();
	}
}

void ASMPlayerCharacter::ClientRPCInterpolationPull_Implementation(FVector_NetQuantize10 InInterpolationLocation)
{
	if (HasAuthority())
	{
		return;
	}

	PullData.bIsPulling = true;

	PullData.StartLocation = GetActorLocation();
	PullData.EndLocation = InInterpolationLocation;

	PullData.ElapsedTime = 0.0f;

	const float CurrentServerTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	PullData.TotalTime = CurrentServerTime - PullData.LastServerTime;
	PullData.LastServerTime = CurrentServerTime;
}

void ASMPlayerCharacter::UpdateInterpolationPull()
{
	if (HasAuthority())
	{
		return;
	}

	// 서버와 클라이언트 간의 호출 순서가 보장되지 않기 때문에 어태치 된 후에도 이 로직이 수행될 수 있습니다. 이런 경우를 방지하는 코드입니다.
	if (GetAttachParentActor())
	{
		PullData.bIsPulling = false;
		return;
	}

	PullData.ElapsedTime += GetWorld()->GetDeltaSeconds();
	const float Alpha = FMath::Clamp(PullData.ElapsedTime / PullData.TotalTime, 0.0f, 1.0f);
	const FVector NewLocation = FMath::Lerp(PullData.StartLocation, PullData.EndLocation, Alpha);

	SetActorLocation(NewLocation);

	if (Alpha >= 1.0f)
	{
		PullData.bIsPulling = false;
	}
}

void ASMPlayerCharacter::HandlePullEnd()
{
	if (HasAuthority())
	{
		NET_LOG(LogSMCharacter, Log, TEXT("당기기 종료"))
		PullData.bIsPulling = false;

		if (!PullData.Caster)
		{
			return;
		}

		NET_LOG(LogSMCharacter, Log, TEXT("어태치 시작"))

		GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = true;
		PullData.Caster->SetCaughtCharacter(this);

		AttachToComponent(PullData.Caster->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale,
		                  TEXT("CatchSocket"));
		StoredSMPlayerController->SetViewTargetWithBlend(PullData.Caster, 0.3f);

		StoredSMAnimInstance->Montage_Stop(0.0f);
		for (const APlayerController* PlayerController : TActorRange<APlayerController>(GetWorld()))
		{
			if (!PlayerController || PlayerController->IsLocalController())
			{
				continue;
			}

			const ASMPlayerCharacter* PlayerCharacter = Cast<ASMPlayerCharacter>(PlayerController->GetPawn());
			if (PlayerCharacter)
			{
				PlayerCharacter->ClientRPCPlayCaughtAnimation(this);
			}
		}
	}
}

void ASMPlayerCharacter::ClientRPCPlayCaughtAnimation_Implementation(ASMPlayerCharacter* InPlayAnimationCharacter) const
{
	if (InPlayAnimationCharacter)
	{
		InPlayAnimationCharacter->StoredSMAnimInstance->PlayCaught();
	}
}

void ASMPlayerCharacter::Smash()
{
	NET_LOG(LogSMCharacter, Log, TEXT("매치기 트리거"));

	if (CaughtCharacter)
	{
		StoredSMAnimInstance->PlaySmash();
		ServerRPCPlaySmashAnimation();
	}
}

void ASMPlayerCharacter::ServerRPCPlaySmashAnimation_Implementation()
{
	SetCurrentState(EPlayerCharacterState::Smash);
	SetCanControl(false);
	StoredSMAnimInstance->PlaySmash();

	for (const ASMPlayerCharacter* CharacterToAnimation : GetCharactersExcludingServerAndCaster())
	{
		if (CharacterToAnimation)
		{
			CharacterToAnimation->ClientRPCPlaySmashAnimation(this);
		}
	}
}

void ASMPlayerCharacter::ClientRPCPlaySmashAnimation_Implementation(
	const ASMPlayerCharacter* InCharacterToAnimation) const
{
	if (InCharacterToAnimation)
	{
		InCharacterToAnimation->StoredSMAnimInstance->PlaySmash();
	}
}

void ASMPlayerCharacter::AnimNotify_Smash()
{
	// 캐스터 클라이언트에서만 실행되야합니다.
	if (!IsLocallyControlled())
	{
		return;
	}

	if (!CaughtCharacter)
	{
		return;
	}

	NET_LOG(LogSMCharacter, Log, TEXT("매치기 처리"));

	CaughtCharacter->StoredSMAnimInstance->PlayKnockDown();
	ServerRPCPlayKnockDownAnimation();

	// 클라이언트에서 먼저 매쳐진 위치로 대상을 이동시킵니다. 이후 서버에도 해당 데이터를 보내 동기화시킵니다.
	// 매쳐지는 위치를 직접 정하는 이유는 애니메이션에만 의존해서 의도된 위치에 매쳐지도록 조정하기 어렵기 때문입니다.
	const FVector DownLocation = GetActorLocation() + (GetActorForwardVector() * 150.0f);
	FRotator DownRotation = FRotationMatrix::MakeFromX(GetActorForwardVector()).Rotator();
	DownRotation.Pitch = 0.0;
	DownRotation.Roll = 0.0;
	ServerRPCDetachToCaster(DownLocation, DownRotation);
}

void ASMPlayerCharacter::ServerRPCPlayKnockDownAnimation_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}

	if (CaughtCharacter)
	{
		CaughtCharacter->SetCurrentState(EPlayerCharacterState::Down);
		CaughtCharacter->StoredSMAnimInstance->PlayKnockDown();
	}

	for (const auto CharacterToAnimation : GetCharactersExcludingServerAndCaster())
	{
		if (CharacterToAnimation)
		{
			CharacterToAnimation->ClientRPCPlayKnockDownAnimation(CaughtCharacter);
		}
	}
}

void ASMPlayerCharacter::ClientRPCPlayKnockDownAnimation_Implementation(ASMPlayerCharacter* InCharacterToAnimation)
{
	if (InCharacterToAnimation)
	{
		InCharacterToAnimation->StoredSMAnimInstance->PlayKnockDown();
	}
}

void ASMPlayerCharacter::ServerRPCDetachToCaster_Implementation(FVector_NetQuantize10 InLocation, FRotator InRotation)
{
	if (!HasAuthority())
	{
		return;
	}

	if (CaughtCharacter)
	{
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, CaughtCharacter.Get(),
		                                &ASMPlayerCharacter::MulticastRPCPlayStandUpVisualEffect,
		                                DesignData->StandUpTime);

		NET_LOG(LogSMCharacter, Log, TEXT("디태치"));
		CaughtCharacter->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		CaughtCharacter->GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = false;
		CaughtCharacter->SetEnableMovement(true);
		CaughtCharacter->SetEnableCollision(true);
		CaughtCharacter->SetCollisionProfileName(CP_KNOCK_DOWN);

		// 뷰타겟 설정은 서버에서만 실행해주면 알아서 동기화됩니다.
		CaughtCharacter->StoredSMPlayerController->SetViewTargetWithBlend(CaughtCharacter, 0.3f);

		// 로컬에서 회전을 변경해주는 이유는 컨트롤러 관련은 언리얼에서 클라이언트 우선이기 때문입니다. 여기서 바꾸는 값은 컨트롤러의 회전값입니다.
		CaughtCharacter->SetActorLocation(InLocation);
		CaughtCharacter->ClientRPCSetRotation(InRotation);

		CaughtCharacter->SmashComponent->TriggerTile(TeamComponent->GetCurrentTeam());

		SetCaughtCharacter(nullptr);
	}
}

void ASMPlayerCharacter::SmashEnded(UAnimMontage* PlayAnimMontage, bool bInterrupted)
{
	if (HasAuthority())
	{
		NET_LOG(LogSMCharacter, Log, TEXT("매치기 애니메이션 종료"))
		SetCurrentState(EPlayerCharacterState::Normal);
		SetCanControl(true);
	}
}
*/

void ASMPlayerCharacter::ResetTeamMaterial()
{
	const int32 TotalMaterialCount = GetMesh()->GetNumMaterials();

	switch (TeamComponent->GetCurrentTeam())
	{
	case ESMTeam::None:
		{
			break;
		}
	case ESMTeam::FutureBass:
		{
			for (int32 i = 0; i < TotalMaterialCount; ++i)
			{
				GetMesh()->SetMaterial(i, AssetData->FutureBassTeamMaterial);
			}

			break;
		}
	case ESMTeam::Rock:
		{
			for (int32 i = 0; i < TotalMaterialCount; ++i)
			{
				GetMesh()->SetMaterial(i, AssetData->RockTeamMaterial);
			}

			break;
		}
	}
}

ESMTeam ASMPlayerCharacter::GetCurrentTeam() const
{
	return TeamComponent->GetCurrentTeam();
}

void ASMPlayerCharacter::ServerRPCFutureBassTeamSelect_Implementation()
{
	TeamComponent->SetTeam(ESMTeam::FutureBass);
}

void ASMPlayerCharacter::ServerRPCRockTeamSelect_Implementation()
{
	TeamComponent->SetTeam(ESMTeam::Rock);
}
