// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectilePool.h"

#include "XyzProjectile.h"

void FProjectilePool::InstantiatePool(UWorld* World, AActor* PoolOwner_In)
{
	if (!ProjectileClass.LoadSynchronous())
	{
		return;
	}

	PoolOwner = PoolOwner_In;
	Projectiles.Reserve(PoolSize);
	for (int i = 0; i < PoolSize; ++i)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = PoolOwner_In;
		AXyzProjectile* Projectile = World->SpawnActor<AXyzProjectile>(ProjectileClass.LoadSynchronous(), PoolWorldLocation, FRotator::ZeroRotator, SpawnParameters);
		Projectiles.Add(Projectile);
	}
}

AXyzProjectile* FProjectilePool::GetNextAvailableProjectile()
{
	if (!IsValid(PoolOwner) || Projectiles.Num() == 0)
	{
		return nullptr;
	}

	int32 ProjectileIndex = CurrentProjectileIndex;
	ProjectileIndex++;
	if (ProjectileIndex >= Projectiles.Num())
	{
		ProjectileIndex = 0;
	}

	if (PoolOwner->GetLocalRole() == ROLE_Authority)
	{
		CurrentProjectileIndex++;
	}

	return Projectiles[ProjectileIndex];
}
