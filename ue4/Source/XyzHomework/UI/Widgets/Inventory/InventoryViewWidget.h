// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "InventoryViewWidget.generated.h"

class UInventorySlot;
class UInventorySlotWidget;
class UGridPanel;

UCLASS()
class XYZHOMEWORK_API UInventoryViewWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void InitializeWidget(const TArray<UInventorySlot*>& InventorySlots);
	void UpdateSlotWidgets(const TArray<UInventorySlot*>& InventorySlots) const;

protected:
	void AddSlotToView(UInventorySlot* SlotToAdd);
	
	UPROPERTY(meta = (BindWidget))
	UGridPanel* ItemSlots;
	UPROPERTY(EditDefaultsOnly, Category = "ItemSlots")
	TSoftClassPtr<UInventorySlotWidget> InventorySlotWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "ItemSlots")
	int32 ColumnCount = 4;

	bool bIsInitialized = false;
};
