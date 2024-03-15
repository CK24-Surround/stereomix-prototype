// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMProjectile.generated.h"

class USMProjectileAssetData;
class UProjectileMovementComponent;
class USphereComponent;

DECLARE_LOG_CATEGORY_CLASS(LogSMProjectile, Log, All);

UCLASS()
class STEREOMIXPROTOTYPE_API ASMProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASMProjectile();

public:
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

// ~Component Section
protected:
	UPROPERTY(VisibleAnywhere, Category = "Collider")
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;
// ~End of Component Section

// ~Data Section
protected:
	UPROPERTY()
	TObjectPtr<const USMProjectileAssetData> AssetData;
// ~End of Data Section

// ~Property Replicate Section
public:
	FORCEINLINE void SetOwningPawn(APawn* NewOwningPawn)
	{
		if (HasAuthority())
		{
			OwningPawn = NewOwningPawn;
			OnRep_OwningPawn();
		}
	}

protected:
	UFUNCTION()
	void OnRep_OwningPawn();

	UPROPERTY(ReplicatedUsing = OnRep_OwningPawn)
	TObjectPtr<APawn> OwningPawn;

// ~End of Property Replicate Section

// ~Event Section
protected:
	UFUNCTION()
	virtual void OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
// ~End of Event Section
};
