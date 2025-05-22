// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponAmmoWidget.h"

void UWeaponAmmoWidget::OnWeaponAmmoChanged(int32 NewAmmo, int32 NewAmmoInEquipment)
{
	WeaponAmmo = NewAmmo;
	WeaponAmmoInEquipment = NewAmmoInEquipment;
}

void UWeaponAmmoWidget::OnThrowableAmmoChanged(int32 NewAmmo)
{
	ThrowableAmmo = NewAmmo;
}
