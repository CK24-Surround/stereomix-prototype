// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/SMProjectile.h"

#include "SMProjectileAssetData.h"
#include "Components/SphereComponent.h"
#include "Data/AssetPath.h"
#include "Design/SMPlayerCharacterDesignData.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Log/SMLog.h"
#include "Net/UnrealNetwork.h"
#include "Physics/SMCollision.h"

ASMProjectile::ASMProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<USMProjectileAssetData> DA_ProjectileAssetRef(PROJECTILE_ASSET_PATH);
	if (DA_ProjectileAssetRef.Succeeded())
	{
		AssetData = DA_ProjectileAssetRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<USMPlayerCharacterDesignData> DA_PlayerCharacterDesignDataRef(PLAYER_CHARACTER_DESIGN_DATA_ASSET_PATH);
	if (DA_PlayerCharacterDesignDataRef.Succeeded())
	{
		DesignData = DA_PlayerCharacterDesignDataRef.Object;
	}
	
	check(AssetData);
	check(DesignData);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionProfileName(CP_PROJECTILE);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(SphereComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (AssetData && AssetData->BaseMesh)
	{
		MeshComponent->SetStaticMesh(AssetData->BaseMesh);
	}

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

	bReplicates = true;

	MaxDistance = DesignData->RangedAttackMaxDistance;
}

void ASMProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	OnActorHit.AddDynamic(this, &ASMProjectile::OnHit);

	if (!HasAuthority())
	{
		OnActorBeginOverlap.AddDynamic(this, &ASMProjectile::OnBeginOverlap);
	}
}

void ASMProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASMProjectile, OwningPawn);
}

void ASMProjectile::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
}

void ASMProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		const float DistanceSquared = FVector::DistSquared(StartLocation, GetActorLocation());
		if (DistanceSquared >= FMath::Square(MaxDistance))
		{
			NET_LOG(LogSMProjectile, Log, TEXT("최대 사거리 도달"))
			Destroy();
		}
	}
}

void ASMProjectile::OnRep_OwningPawn()
{
	NET_LOG(LogSMProjectile, Log, TEXT("\"%s\"로 오너 설정"), *OwningPawn->GetName());
	SetOwner(OwningPawn);
}

void ASMProjectile::OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	NET_LOG(LogSMProjectile, Log, TEXT("벽 충돌"))
	Destroy();
}

void ASMProjectile::OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor) {}
