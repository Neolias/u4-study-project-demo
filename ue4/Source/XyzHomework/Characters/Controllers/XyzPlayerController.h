// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "XyzPlayerController.generated.h"

class UPlayerHUDWidget;

UCLASS()
class XYZHOMEWORK_API AXyzPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetPawn(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;
	bool IgnoresFPCameraPitch() const;
	void SetIgnoreFPCameraPitch(bool bIgnoreFPCameraPitch_In) { bIgnoresFPCameraPitch = bIgnoreFPCameraPitch_In; }

protected:
	virtual void SetupInputComponent() override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSoftClassPtr<UPlayerHUDWidget> PlayerHUDWidgetClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSoftClassPtr<UUserWidget> MainMenuWidgetClass;

private:
	void CreateAndInitializeHUDWidgets();
	void RemoveHUDWidgets() const;
	void ToggleMainMenu();
	/** Finds an input key assigned to this 'ActionName' action and shows it in the player's HUD. */
	void OnInteractableObjectFound(FName ActionName);

#pragma region INPUT ACTIONS
	
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void Crouch();
	void UnCrouch();
	void Jump();
	void StartSprint();
	void StopSprint();
	void ToggleProne();
	void Mantle();
	void SwimForward(float Value);
	void SwimRight(float Value);
	void SwimUp(float Value);
	void Dive();
	void ClimbLadderUp(float Value);
	void UseEnvironmentActor();
	void JumpOffRunnableWall();
	void Slide();
	void ReloadLevel();
	void StartWeaponFire();
	void StopFire();
	void StartAim();
	void EndAim();
	void ReloadWeapon();
	void DrawNextEquipmentItem();
	void DrawPreviousEquipmentItem();
	void TogglePrimaryItem();
	void ThrowItem();
	void ActivateNextWeaponMode();
	void UsePrimaryMeleeAttack();
	void UseSecondaryMeleeAttack();
	void InteractWithObject();
	void UseInventory();
	void UseRadialMenu();
	void QuickSaveGame();
	void QuickLoadGame();
	void TogglePlayerMouseInput();
	void QuitGame();
#pragma endregion

	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);

	TWeakObjectPtr<class AXyzBaseCharacter> CachedBaseCharacter;
	bool bIgnoresFPCameraPitch = false;
	UPROPERTY()
	UPlayerHUDWidget* PlayerHUDWidget;
	UPROPERTY()
	UUserWidget* MainMenuWidget;
};
