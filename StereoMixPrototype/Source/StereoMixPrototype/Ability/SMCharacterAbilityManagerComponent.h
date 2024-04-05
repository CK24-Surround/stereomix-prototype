// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "SMCharacterAbilityComponent.h"
#include "Components/ActorComponent.h"
#include "SMCharacterAbilityManagerComponent.generated.h"

class ISMCharacterAbilityInterface;

UCLASS(ClassGroup=(SMAbility), Blueprintable, meta=(BlueprintSpawnableComponent))
class STEREOMIXPROTOTYPE_API USMCharacterAbilityManagerComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Ability", meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<USMCharacterAbilityComponent>> Abilities;

public:
	// Sets default values for this component's properties
	USMCharacterAbilityManagerComponent();

	virtual void InitializeComponent() override;

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	ISMCharacterAbilityInterface* GetAbility(const FName& AbilityName) const;

	FORCEINLINE const TMap<FName, TObjectPtr<USMCharacterAbilityComponent>>& GetAbilities() const { return Abilities; }

	template <class T> requires sm_ability<T>
	T* AddAbility(FName AbilityName)
	{
		T* NewAbility = GetOwner()->CreateDefaultSubobject<T>(AbilityName);
		Abilities.Add(AbilityName, CastChecked<USMCharacterAbilityComponent>(NewAbility));
		return NewAbility;
	}

protected:
};
