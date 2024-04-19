// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlatformInvocator.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInvocatorActivated);

UCLASS()
class XYZHOMEWORK_API APlatformInvocator : public AActor
{
	GENERATED_BODY()
	
public:	
	APlatformInvocator();
	UFUNCTION(BlueprintCallable)
	void Invoke() const;
	FOnInvocatorActivated OnInvocatorActivated;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaSeconds) override;
};
