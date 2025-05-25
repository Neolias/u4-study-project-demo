// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Actors/Interactive/Interactable.h"
#include "GameFramework/Actor.h"
#include "PickupItem.generated.h"

/** Base class of all pickable items, such as consumables, weapons, ammo, etc. */
UCLASS()
class XYZHOMEWORK_API APickupItem : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	APickupItem();
	virtual FName GetActionName() override;
	virtual EInventoryItemType GetItemType() const { return ItemType; }
	virtual void Interact(APawn* InteractingPawn) override;
	virtual void SetAmount(int32 NewAmount) { Amount = NewAmount; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable")
	EInventoryItemType ItemType = EInventoryItemType::None;
	/** Size of a stack of inventory items this object represents. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable", meta = (ClampMin = 1, UIMin = 1))
	int32 Amount = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	UStaticMeshComponent* ItemMesh;
	/** Name of the input action used to interact with this object (see input component actions). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	FName ActionName = FName("InteractWithObject");
};
