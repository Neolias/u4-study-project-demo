// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Items/InventoryItem.h"
#include "HealthPackInventoryItem.generated.h"

UCLASS()
class XYZHOMEWORK_API UHealthPackInventoryItem : public UInventoryItem
{
	GENERATED_BODY()

public:
	virtual bool Consume(APawn* Pawn) override;

private:
	float HealthRestoreAmount = 25.f;
	
};
