// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/SMProjectile.h"

#include "SMProjectileAssetData.h"
#include "Components/SphereComponent.h"
#include "Data/AssetPath.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Log/SMLog.h"
#include "Net/UnrealNetwork.h"

ASMProjectile::ASMProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<USMProjectileAssetData> DA_ProjectileAssetRef(PROJECTILE_ASSET_PATH);
	if (DA_ProjectileAssetRef.Succeeded())
	{
		AssetData = DA_ProjectileAssetRef.Object;
	}

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SetRootComponent(SphereComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(SphereComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (AssetData && AssetData->BaseMesh)
	{
		MeshComponent->SetStaticMesh(AssetData->BaseMesh);
	}

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
}

void ASMProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		SetActorEnableCollision(false);
	}
	else
	{
		OnActorBeginOverlap.AddDynamic(this, &ASMProjectile::OnBeginOverlap);
	}
}

void ASMProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ASMProjectile, OwningPawn);
}

void ASMProjectile::OnRep_OwningPawn()
{
	NET_LOG(LogSMProjectile, Log, TEXT("\"%s\"로 오너 설정"), *OwningPawn->GetName());
	SetOwner(OwningPawn);
}

void ASMProjectile::OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor) {}
