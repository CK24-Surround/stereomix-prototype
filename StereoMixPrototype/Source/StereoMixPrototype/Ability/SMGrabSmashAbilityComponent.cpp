// Fill out your copyright notice in the Description page of Project Settings.


#include "SMGrabSmashAbilityComponent.h"

#include "Animation/SMCharacterAnimationAssetData.h"
#include "Animation/SMCharacterAnimInstance.h"
#include "Log/SMLog.h"

USMGrabSmashAbilityComponent::USMGrabSmashAbilityComponent()
{
	TriggerOption = ETriggerEvent::Started;
	DefaultCooldown = 1.0f;
	PerformDelay = 0.0f;
	bIsMovableAbility = false;

	GrabRadius = 100.0f;
}

void USMGrabSmashAbilityComponent::PostInitializeOwner()
{
	Super::PostInitializeOwner();
}

void USMGrabSmashAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	// StoredSMAnimInstance가 PostInitializeOwner 이후에 초기화됨
	// ActionMontage = GetOwnerCharacter()->GetStoredSMAnimInstance()->GetAssetData()->DashGrabMontage;
	// SmashMontage = GetOwnerCharacter()->GetStoredSMAnimInstance()->GetAssetData()->GrabSmashMontage;
}

void USMGrabSmashAbilityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	NET_COMP_LOG(LogSMAbility_GrabSmash, Log, TEXT("EndPlay"));
}

void USMGrabSmashAbilityComponent::TickComponent(const float DeltaTime, const ELevelTick TickType,
                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void USMGrabSmashAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool USMGrabSmashAbilityComponent::CanAction() const
{
	const EPlayerCharacterState CurrentState = GetOwnerCharacter()->GetCurrentState();
	if (CurrentState != EPlayerCharacterState::Normal)
	{
		return false;
	}

	NET_COMP_LOG(LogSMAbility_GrabSmash, Log, TEXT("CanAction"));
	return Super::CanAction();
}

void USMGrabSmashAbilityComponent::OnServerPerformAbility(const double ClientDelay)
{
	Super::OnServerPerformAbility(ClientDelay);

	CurrentCooldown = DefaultCooldown;
}

void USMGrabSmashAbilityComponent::OnPlayVisualEffectsOnAction() const
{
	Super::OnPlayVisualEffectsOnAction();
}

void USMGrabSmashAbilityComponent::OnPlayVisualEffectsOnPerform() const
{
	Super::OnPlayVisualEffectsOnPerform();
}
