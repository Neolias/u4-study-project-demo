// Fill out your copyright notice in the Description page of Project Settings.


#include "ReticleWidget.h"

#include "Actors/Equipment/EquipmentItem.h"

void UReticleWidget::OnAimingStateChanged_Implementation(bool bIsAiming)
{
	const AEquipmentItem* EquippedItem = CurrentEquippedItem.Get();
	CurrentReticleType = IsValid(EquippedItem) ? (bIsAiming ? EquippedItem->GetAimingReticleType() : EquippedItem->GetReticleType()) : EReticleType::None;
}

void UReticleWidget::OnEquippedItemChanged_Implementation(const AEquipmentItem* EquippedItem)
{
	CurrentEquippedItem = EquippedItem;
	CurrentReticleType = IsValid(EquippedItem) ? EquippedItem->GetReticleType() : EReticleType::None;
}
