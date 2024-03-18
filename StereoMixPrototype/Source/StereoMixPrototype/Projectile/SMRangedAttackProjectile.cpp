// Fill out your copyright notice in the Description page of Project Settings.


#include "SMRangedAttackProjectile.h"

#include "Design/SMPlayerCharacterDesignData.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interface/SMProjectileInterface.h"
#include "Log/SMLog.h"


ASMRangedAttackProjectile::ASMRangedAttackProjectile()
{
	ProjectileMovementComponent->SetAutoActivate(false);
}

void ASMRangedAttackProjectile::BeginPlay()
{
	Super::BeginPlay();
	ProjectileMovementComponent->Activate(true);
	ProjectileMovementComponent->Velocity = GetActorForwardVector() * DesignData->RangedAttackProjectileSpeed;
}

void ASMRangedAttackProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASMRangedAttackProjectile::OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	Super::OnBeginOverlap(OverlappedActor, OtherActor);

	const ISMProjectileInterface* ProjectileInterface = Cast<ISMProjectileInterface>(OtherActor);
	if (ProjectileInterface)
	{
		// 자기 자신이 발사한 투사체는 무시합니다.
		if (OwningPawn == OtherActor)
		{
			return;
		}
		
		if (OwningPawn && OwningPawn->IsLocallyControlled())
		{
			ServerRPCHitProjectile(OtherActor, GetActorLocation());
		}

		SetActorEnableCollision(false);
		SetActorHiddenInGame(true);
	}
}

void ASMRangedAttackProjectile::ServerRPCHitProjectile_Implementation(AActor* HitActor, FVector_NetQuantize10 InLocation)
{
	ISMProjectileInterface* ProjectileInterface = Cast<ISMProjectileInterface>(HitActor);
	if (ProjectileInterface)
	{
		const float Distance = FVector::Dist(InLocation, GetActorLocation());
		NET_LOG(LogSMProjectile, Log, TEXT("서버와 클라이언트의 위치 차이: %f"), Distance);
		ProjectileInterface->HitProjectile();
	}
	
	Destroy();
}
