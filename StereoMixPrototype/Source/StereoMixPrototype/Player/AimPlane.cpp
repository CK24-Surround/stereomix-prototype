// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AimPlane.h"

#include "Physics/SMCollision.h"

AAimPlane::AAimPlane()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootComponent);
	
	AimPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AimPlane"));
	AimPlane->SetupAttachment(GetRootComponent());
	AimPlane->SetCollisionProfileName(CP_AIM_PLANE);
	AimPlane->SetRelativeScale3D(FVector(100.0));
	AimPlane->SetVisibility(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshRef(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
	if (PlaneMeshRef.Succeeded())
	{
		AimPlane->SetStaticMesh(PlaneMeshRef.Object);
	}
}

void AAimPlane::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAimPlane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

