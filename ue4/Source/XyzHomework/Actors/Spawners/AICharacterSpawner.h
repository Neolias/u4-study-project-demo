// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AICharacterSpawner.generated.h"


UCLASS()
class XYZHOMEWORK_API AAICharacterSpawner : public AActor
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void SpawnAI();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "AISpawner"))
	bool bSpawnAtBeginPlay = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "AISpawner"))
	bool bSpawnOnce = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "AISpawner"))
	TSoftClassPtr<class AAICharacter> AICharacterClass;
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, meta = (Category = "AISpawner", ToolTip = "IInteractable object"))
	AActor* SpawnTriggerActor;
	
private:
	void UpdateSpawnTrigger();
	void UnsubscribeFromTrigger() const;

	TScriptInterface<class IInteractable> SpawnTrigger;
	bool bCanSpawn = true;
	FDelegateHandle OnSpawnDelegate;
};
