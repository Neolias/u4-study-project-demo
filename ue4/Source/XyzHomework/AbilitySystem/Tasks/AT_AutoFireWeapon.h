// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Tasks/AT_TickTaskBase.h"
#include "AT_AutoFireWeapon.generated.h"

class ARangedWeaponItem;
/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UAT_AutoFireWeapon : public UAT_TickTaskBase
{
	GENERATED_BODY()

public:
	virtual void TickTask(float DeltaTime) override;
	static UAT_AutoFireWeapon* NewAutoFireWeaponTask(UGameplayAbility* OwningAbility);
	virtual void Activate() override;

private:
	TWeakObjectPtr<ARangedWeaponItem> CachedRangedWeapon;
};
