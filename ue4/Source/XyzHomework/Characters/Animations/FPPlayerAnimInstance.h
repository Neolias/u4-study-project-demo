// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzBaseCharacterAnimInstance.h"
#include "FPPlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UFPPlayerAnimInstance : public UXyzBaseCharacterAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;

private:
	TWeakObjectPtr<class AFPPlayerCharacter> CachedFPCharacter;
};
