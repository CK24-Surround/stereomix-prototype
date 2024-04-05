// Fill out your copyright notice in the Description page of Project Settings.


#include "SMCharacterAbilityComponent.h"

#include "EnhancedInputComponent.h"
#include "Animation/SMCharacterAnimInstance.h"
#include "GameFramework/GameStateBase.h"
#include "Log/SMLog.h"
#include "Net/UnrealNetwork.h"

DECLARE_LOG_CATEGORY_CLASS(LogSMAbility, Log, All);

USMCharacterAbilityComponent::USMCharacterAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);

	AbilityName = GetFName();
	OnActionMontageEndedDelegate.BindUObject(this, &USMCharacterAbilityComponent::OnActionMontageEnded);
	TriggerOption = ETriggerEvent::Started;
	CooldownTickRate = 0.1f;
}

void USMCharacterAbilityComponent::BindInput(UEnhancedInputComponent* InputComponent, const UInputAction* InputAction)
{
	if (InputComponent)
	{
		check(InputAction);
		InputComponent->BindAction(InputAction, TriggerOption, this, &USMCharacterAbilityComponent::UseAbility);
		NET_COMP_LOG(LogSMAbility, Log, TEXT("<%s> Bind Input Action: %s"), *GetAbilityName().ToString(),
		             *InputAction->GetName());
	}
}

void USMCharacterAbilityComponent::PostInitializeOwner()
{
	OwnerCharacter = CastChecked<ASMPlayerCharacter>(GetOwner());

	NET_COMP_LOG(LogSMAbility, Log, TEXT("<%s> Owner initialized."), *GetAbilityName().ToString());
}

// Called when the game starts
void USMCharacterAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USMCharacterAbilityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void USMCharacterAbilityComponent::TickComponent(const float DeltaTime, const ELevelTick TickType,
                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateCooldownOnTick(DeltaTime);
}

void USMCharacterAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USMCharacterAbilityComponent, OwnerCharacter);
	DOREPLIFETIME_CONDITION(USMCharacterAbilityComponent, CurrentCooldown, COND_OwnerOnly);
}

void USMCharacterAbilityComponent::OnRep_IsActivate()
{
}

void USMCharacterAbilityComponent::OnRep_CurrentCooldown()
{
}

void USMCharacterAbilityComponent::UseAbility()
{
	if (!OwnerCharacter->IsLocallyControlled())
	{
		return;
	}
	if (!CanAction())
	{
		return;
	}
	UAnimMontage* Montage = GetActionMontage();
	if (!Montage)
	{
		NET_COMP_LOG(LogSMAbility, Error, TEXT("Can not found Action Montage."));
		return;
	}
	const float PlayResult = OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Play(Montage);
	if (PlayResult == 0.0f)
	{
		return;
	}
	OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(OnActionMontageEndedDelegate, Montage);
	OnPlayVisualEffectsOnAction();

	if (!IsMovableAbility())
	{
		OwnerCharacter->DisableInput(OwnerCharacter->GetLocalViewingPlayerController());
		OwnerCharacter->bUseControllerRotationPitch = false;
		OwnerCharacter->bUseControllerRotationYaw = false;
		OwnerCharacter->bUseControllerRotationRoll = false;
	}

	bIsActivate = true;
	NET_COMP_LOG(LogSMAbility, Log, TEXT("<%s> Use ability."), *GetAbilityName().ToString());
	ServerPerformAbility(GetWorld()->GetGameState()->GetServerWorldTimeSeconds());
}

bool USMCharacterAbilityComponent::ServerPerformAbility_Validate(const double Timestamp)
{
	// 딜레이가 음수가 나온다는 것은 있을 수 없는 일입니다.
	if (GetWorld()->GetGameState()->GetServerWorldTimeSeconds() - Timestamp < 0.0f)
	{
		return false;
	}

	return true;
}

void USMCharacterAbilityComponent::ServerPerformAbility_Implementation(const double Timestamp)
{
	NET_COMP_LOG(LogSMAbility, Log, TEXT("<%s> ServerPerformAbility."), *GetAbilityName().ToString());
#if WITH_SERVER_CODE
	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}

	// 추후 서버 되감기 수행하는 코드 추가
	if (!CanAction())
	{
		return;
	}

	if (UAnimMontage* Montage = GetActionMontage())
	{
		const float PlayResult = OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Play(Montage);
		if (PlayResult == 0.0f)
		{
			return;
		}
		OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(OnActionMontageEndedDelegate, Montage);
		MulticastPlayVisualEffects();
		MulticastPlayVisualEffectsOnPerform();
	}

	OnServerPerformAbility(GetWorld()->GetGameState()->GetServerWorldTimeSeconds() - Timestamp);
#endif
}

void USMCharacterAbilityComponent::ServerCancelAbility_Implementation()
{
	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}
	CancelImmediate();
}

bool USMCharacterAbilityComponent::CanAction() const
{
	if (CurrentCooldown > 0.0f)
	{
		return false;
	}
	return true;
}

void USMCharacterAbilityComponent::CancelImmediate()
{
	if (!IsActivate())
	{
		return;
	}

	if (OwnerCharacter->IsLocallyControlled())
	{
		ServerCancelAbility();
	}
	OnCancelAbility();
	OnEndAbility(true);
}

void USMCharacterAbilityComponent::OnServerPerformAbility(const double ClientDelay)
{
	NET_COMP_LOG(LogSMAbility, Log, TEXT("<%s> OnServerPerformAbility"), *GetAbilityName().ToString());
#if WITH_SERVER_CODE
	// ...
#endif
}

void USMCharacterAbilityComponent::OnEndAbility(bool bIsCanceled)
{
	NET_COMP_LOG(LogSMAbility, Log, TEXT("<%s> OnEndAbility"), *GetAbilityName().ToString());
	if (OwnerCharacter->IsLocallyControlled())
	{
		if (!IsMovableAbility())
		{
			OwnerCharacter->EnableInput(OwnerCharacter->GetLocalViewingPlayerController());
			OwnerCharacter->bUseControllerRotationPitch = true;
			OwnerCharacter->bUseControllerRotationYaw = true;
			OwnerCharacter->bUseControllerRotationRoll = true;
		}
	}

	bIsActivate = false;
}

void USMCharacterAbilityComponent::OnCancelAbility()
{
	NET_COMP_LOG(LogSMAbility, Log, TEXT("<%s> OnCancelAbility"), *GetAbilityName().ToString());
}

void USMCharacterAbilityComponent::MulticastPlayVisualEffects_Implementation()
{
	if (GetOwnerCharacter()->GetLocalRole() != ROLE_SimulatedProxy)
	{
		return;
	}

	if (UAnimMontage* Montage = GetActionMontage())
	{
		OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Play(Montage);
	}
	OnPlayVisualEffectsOnAction();
}

void USMCharacterAbilityComponent::OnPlayVisualEffectsOnAction() const
{
}

void USMCharacterAbilityComponent::MulticastPlayVisualEffectsOnPerform_Implementation()
{
	if (!GetOwnerCharacter()->IsLocallyControlled() && GetOwnerCharacter()->GetLocalRole() != ROLE_SimulatedProxy)
	{
		return;
	}
	OnPlayVisualEffectsOnPerform();
}

void USMCharacterAbilityComponent::OnPlayVisualEffectsOnPerform() const
{
}

void USMCharacterAbilityComponent::UpdateCooldownOnTick(const float DeltaTime)
{
	if (CurrentCooldown == 0.0f)
	{
		return;
	}
	CurrentCooldown = FMath::Max(CurrentCooldown - DeltaTime, 0.0f);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void USMCharacterAbilityComponent::OnActionMontageEnded(UAnimMontage* Montage, const bool bInterrupted)
{
	NET_COMP_LOG(LogSMAbility, Log, TEXT("<%s> OnActionMontageEnded"), *GetAbilityName().ToString());
	if (Montage != GetActionMontage())
	{
		return;
	}
	if (bInterrupted)
	{
		OnCancelAbility();
	}
	OnEndAbility(bInterrupted);
}
