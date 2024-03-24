// Fill out your copyright notice in the Description page of Project Settings.


#include "SMRangedAttackProjectile.h"

#include "NiagaraFunctionLibrary.h"
#include "SMProjectileAssetData.h"
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

	const ISMProjectileInterface* OtherActorProjectileInterface = Cast<ISMProjectileInterface>(OtherActor);
	if (OtherActorProjectileInterface)
	{
		// 같은 팀은 무시합니다.
		const ISMProjectileInterface* OwningActorProjectileInterface = Cast<ISMProjectileInterface>(OwningPawn);
		if (OwningActorProjectileInterface)
		{
			// 만약 아무 팀도 선택되지 않은 캐릭터는 모두가 무시합니다.
			if ((OwningActorProjectileInterface->GetCurrentTeam() == ESMTeam::None) || (OtherActorProjectileInterface->GetCurrentTeam() == ESMTeam::None))
			{
				return;
			}

			if (OtherActorProjectileInterface->GetCurrentTeam() == OwningActorProjectileInterface->GetCurrentTeam())
			{
				return;
			}
		}

		// 서버가 아닌 투사체를 발사한 클라이언트에서 충돌 판정 계산하고 이를 사용하기 위한 코드입니다.
		if (OwningPawn && OwningPawn->IsLocallyControlled())
		{
			ServerRPCHitProjectile(OtherActor, GetActorLocation());
		}

		SetActorEnableCollision(false);
		SetActorHiddenInGame(true);
	}
}

void ASMRangedAttackProjectile::OnRep_OwningPawn()
{
	Super::OnRep_OwningPawn();

	if (!HasAuthority())
	{
		const ISMProjectileInterface* ProjectileInterface = Cast<ISMProjectileInterface>(OwningPawn);
		if (ProjectileInterface)
		{
			switch (ProjectileInterface->GetCurrentTeam())
			{
				case ESMTeam::None:
					break;
				case ESMTeam::FutureBass:
				{
					MeshComponent->SetMaterial(0, AssetData->FutureBassTeamMaterial);
					break;
				}
				case ESMTeam::Rock:
				{
					MeshComponent->SetMaterial(0, AssetData->RockTeamMaterial);
					break;
				}
			}
		}
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
