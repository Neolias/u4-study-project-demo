// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "AbilitySystem/Tasks/AT_AutoFireWeapon.h"

#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void UAT_AutoFireWeapon::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	ARangedWeaponItem* RangedWeapon = CachedRangedWeapon.Get();
	if (IsValid(RangedWeapon))
	{
		RangedWeapon->StartFire();
	}
}

UAT_AutoFireWeapon* UAT_AutoFireWeapon::NewAutoFireWeaponTask(UGameplayAbility* OwningAbility)
{
	UAT_AutoFireWeapon* MyObj = NewAbilityTask<UAT_AutoFireWeapon>(OwningAbility);
	return MyObj;
}

void UAT_AutoFireWeapon::Activate()
{
	Super::Activate();

	const AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		CachedRangedWeapon = CachedBaseCharacter->GetCharacterEquipmentComponent()->GetCurrentRangedWeapon();
	}
}
