// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

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
