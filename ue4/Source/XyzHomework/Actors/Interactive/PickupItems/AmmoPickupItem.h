// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactive/PickupItems/PickupItem.h"
#include "AmmoPickupItem.generated.h"

UCLASS()
class XYZHOMEWORK_API AAmmoPickupItem : public APickupItem
{
	GENERATED_BODY()

public:
	virtual void Interact(APawn* InteractingPawn) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Ammo")
	EWeaponAmmoType AmmoType = EWeaponAmmoType::None;

};
