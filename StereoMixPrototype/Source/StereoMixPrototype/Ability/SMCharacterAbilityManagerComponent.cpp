#include "SMCharacterAbilityManagerComponent.h"


#include "SMGrabSmashAbilityComponent.h"
#include "Interface/SMCharacterAbilityInterface.h"

USMCharacterAbilityManagerComponent::USMCharacterAbilityManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Abilities = TMap<FName, TObjectPtr<USMCharacterAbilityComponent>>();
}

void USMCharacterAbilityManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void USMCharacterAbilityManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USMCharacterAbilityManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void USMCharacterAbilityManagerComponent::TickComponent(const float DeltaTime, const ELevelTick TickType,
                                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

ISMCharacterAbilityInterface* USMCharacterAbilityManagerComponent::GetAbility(const FName& AbilityName) const
{
	ISMCharacterAbilityInterface* TargetAbility = Abilities.FindRef(AbilityName);
	checkf(TargetAbility, TEXT("Ability not found."));
	return TargetAbility;
}
