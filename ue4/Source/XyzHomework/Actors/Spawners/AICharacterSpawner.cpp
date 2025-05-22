// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Spawners/AICharacterSpawner.h"

#include "Actors/Interactive/Interactable.h"
#include "AI/Characters/AICharacter.h"

void AAICharacterSpawner::SpawnAI()
{
	if (!bCanSpawn || !AICharacterClass.LoadSynchronous() || GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	AAICharacter* AICharacter = GetWorld()->SpawnActor<AAICharacter>(AICharacterClass.LoadSynchronous(), GetTransform());
	if (!IsValid(AICharacter->GetController()))
	{
		AICharacter->SpawnDefaultController();
	}

	if (bSpawnOnce)
	{
		bCanSpawn = false;
	}
}

void AAICharacterSpawner::BeginPlay()
{
	Super::BeginPlay();

	UpdateSpawnTrigger();

	if (bSpawnAtBeginPlay)
	{
		SpawnAI();
		UnsubscribeFromTrigger();
	}
}

void AAICharacterSpawner::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UnsubscribeFromTrigger();
}

#if WITH_EDITOR
void AAICharacterSpawner::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UpdateSpawnTrigger();
}
#endif // WITH_EDITOR

void AAICharacterSpawner::UpdateSpawnTrigger()
{
	if (SpawnTrigger == SpawnTriggerActor)
	{
		return;
	}

	SpawnTrigger = SpawnTriggerActor;
	if (!IsValid(SpawnTriggerActor) || !SpawnTrigger.GetInterface())
	{
		SpawnTriggerActor = nullptr;
		SpawnTrigger = nullptr;
	}

	if (SpawnTrigger->HasOnInteractionCallback())
	{
		OnSpawnDelegate = SpawnTrigger->AddOnInteractionDelegate(this, FName("SpawnAI"));
	}
}

void AAICharacterSpawner::UnsubscribeFromTrigger() const
{
	if (OnSpawnDelegate.IsValid() && SpawnTrigger.GetInterface())
	{
		SpawnTrigger->RemoveOnInteractionDelegate(OnSpawnDelegate);
	}
}
