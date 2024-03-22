// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SMProjectile.h"
#include "SMRangedAttackProjectile.generated.h"

UCLASS()
class STEREOMIXPROTOTYPE_API ASMRangedAttackProjectile : public ASMProjectile
{
	GENERATED_BODY()

public:
	ASMRangedAttackProjectile();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

// ~Event Section
protected:
	virtual void OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor) override;
// ~End of Event Section

protected:
	virtual void OnRep_OwningPawn() override;

protected:
	UFUNCTION(Server, Reliable)
	void ServerRPCHitProjectile(AActor* HitActor, FVector_NetQuantize10 InLocation);
};
