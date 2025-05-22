// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponAmmoWidget.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UWeaponAmmoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void OnWeaponAmmoChanged(int32 NewAmmo, int32 NewAmmoInEquipment);
	void OnThrowableAmmoChanged(int32 NewAmmo);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 WeaponAmmo = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 WeaponAmmoInEquipment = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 ThrowableAmmo = 0;
};
