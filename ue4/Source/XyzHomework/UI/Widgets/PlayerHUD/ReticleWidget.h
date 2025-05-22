// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Blueprint/UserWidget.h"
#include "ReticleWidget.generated.h"

class AEquipmentItem;
/**
 *
 */
UCLASS()
class XYZHOMEWORK_API UReticleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void OnAimingStateChanged(bool bIsAiming);
	UFUNCTION(BlueprintNativeEvent)
	void OnEquippedItemChanged(const AEquipmentItem* EquippedItem);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reticle")
	EReticleType CurrentReticleType = EReticleType::None;

private:
	void OnAimingStateChanged_Implementation(bool bIsAiming);
	void OnEquippedItemChanged_Implementation(const AEquipmentItem* EquippedItem);

	TWeakObjectPtr<const AEquipmentItem> CurrentEquippedItem;
};
