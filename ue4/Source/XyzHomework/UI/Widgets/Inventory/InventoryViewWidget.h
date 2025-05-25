// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryViewWidget.generated.h"

class UGridPanel;
class UInventorySlot;
class UInventorySlotWidget;

/** Widget visualizing the character inventory. */
UCLASS()
class XYZHOMEWORK_API UInventoryViewWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void InitializeWidget(const TArray<UInventorySlot*>& InventorySlots);
	void UpdateSlotWidgets(const TArray<UInventorySlot*>& InventorySlots) const;

protected:
	void AddSlotToView(UInventorySlot* SlotToAdd);

	/** All inventory slot widgets owned by this widget. */
	UPROPERTY(meta = (BindWidget))
	UGridPanel* ItemSlots;
	UPROPERTY(EditDefaultsOnly, Category = "Inventory View Widget")
	TSoftClassPtr<UInventorySlotWidget> InventorySlotWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "Inventory View Widget")
	int32 ColumnCount = 4;

	bool bIsInitialized = false;
};
