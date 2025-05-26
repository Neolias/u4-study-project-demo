// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "Characters/Controllers/XyzPlayerController.h"

#include "SignificanceManager.h"
#include "AbilitySystem/AttributeSets/XyzCharacterAttributeSet.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "GameFramework/PlayerInput.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/SaveSubsystem/SaveSubsystem.h"
#include "UI/Widgets/PlayerHUD/CharacterAttributesWidget.h"
#include "UI/Widgets/PlayerHUD/PlayerHUDWidget.h"
#include "UI/Widgets/PlayerHUD/ReticleWidget.h"
#include "UI/Widgets/PlayerHUD/WeaponAmmoWidget.h"

void AXyzPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (BaseCharacter)
	{
		BaseCharacter->OnInteractiveObjectFound.RemoveAll(this);
		RemoveHUDWidgets();
	}

	if (!IsValid(InPawn))
	{
		CachedBaseCharacter.Reset();
		return;
	}

	checkf(InPawn->IsA<AXyzBaseCharacter>(), TEXT("AXyzPlayerController::SetPawn(): AXyzPlayerController can only be used with AXyzBaseCharacter."))
	BaseCharacter = StaticCast<AXyzBaseCharacter*>(InPawn);
	CachedBaseCharacter = BaseCharacter;
	if (BaseCharacter && IsLocalController())
	{
		BaseCharacter->OnInteractiveObjectFound.AddUObject(this, &AXyzPlayerController::OnInteractableObjectFound);
		CreateAndInitializeHUDWidgets();
	}
}

void AXyzPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (BaseCharacter)
	{
		BaseCharacter->OnInteractiveObjectFound.RemoveAll(this);
	}
	CachedBaseCharacter.Reset();
}

void AXyzPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (USignificanceManager* SignificanceManager = FSignificanceManagerModule::Get(GetWorld()))
	{
		FVector ViewLocation;
		FRotator ViewRotation;
		GetPlayerViewPoint(ViewLocation, ViewRotation);
		FTransform ViewTransform(ViewRotation, ViewLocation);
		TArray<FTransform> ViewPoints = {ViewTransform};
		SignificanceManager->Update(ViewPoints);
	}
}

bool AXyzPlayerController::IgnoresFPCameraPitch() const
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		return !BaseCharacter->IsFirstPerson() || bIgnoresFPCameraPitch;
	}
	return true;
}

void AXyzPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAxis("MoveForward", this, &AXyzPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AXyzPlayerController::MoveRight);
	InputComponent->BindAxis("Turn", this, &AXyzPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &AXyzPlayerController::LookUp);
	InputComponent->BindAction("UseEnvironmentActor", EInputEvent::IE_Pressed, this, &AXyzPlayerController::UseEnvironmentActor);
	InputComponent->BindAction("Mantle", EInputEvent::IE_Pressed, this, &AXyzPlayerController::Mantle);
	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AXyzPlayerController::Jump);
	InputComponent->BindAction("Prone", EInputEvent::IE_Pressed, this, &AXyzPlayerController::ToggleProne);
	InputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &AXyzPlayerController::Crouch);
	InputComponent->BindAction("UnCrouch", EInputEvent::IE_Pressed, this, &AXyzPlayerController::UnCrouch);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &AXyzPlayerController::StartSprint);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &AXyzPlayerController::StopSprint);
	InputComponent->BindAxis("SwimForward", this, &AXyzPlayerController::SwimForward);
	InputComponent->BindAxis("SwimRight", this, &AXyzPlayerController::SwimRight);
	InputComponent->BindAction("Dive", EInputEvent::IE_Pressed, this, &AXyzPlayerController::Dive);
	InputComponent->BindAxis("SwimUp", this, &AXyzPlayerController::SwimUp);
	InputComponent->BindAxis("ClimbLadderUp", this, &AXyzPlayerController::ClimbLadderUp);
	InputComponent->BindAction("JumpOffRunnableWall", EInputEvent::IE_Pressed, this, &AXyzPlayerController::JumpOffRunnableWall);
	InputComponent->BindAxis("WallRun", this, &AXyzPlayerController::WallRun);
	InputComponent->BindAction("Slide", EInputEvent::IE_Pressed, this, &AXyzPlayerController::Slide);
	InputComponent->BindAction("ReloadLevel", EInputEvent::IE_Pressed, this, &AXyzPlayerController::ReloadLevel);
	InputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &AXyzPlayerController::StartWeaponFire);
	InputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &AXyzPlayerController::StopFire);
	InputComponent->BindAction("Aim", EInputEvent::IE_Pressed, this, &AXyzPlayerController::StartAim);
	InputComponent->BindAction("Aim", EInputEvent::IE_Released, this, &AXyzPlayerController::EndAim);
	InputComponent->BindAction("ReloadWeapon", EInputEvent::IE_Pressed, this, &AXyzPlayerController::ReloadWeapon);
	InputComponent->BindAction("DrawNextItem", EInputEvent::IE_Pressed, this, &AXyzPlayerController::DrawNextEquipmentItem);
	InputComponent->BindAction("DrawPreviousItem", EInputEvent::IE_Pressed, this, &AXyzPlayerController::DrawPreviousEquipmentItem);
	InputComponent->BindAction("TogglePrimaryItem", EInputEvent::IE_Pressed, this, &AXyzPlayerController::TogglePrimaryItem);
	InputComponent->BindAction("ThrowItem", EInputEvent::IE_Pressed, this, &AXyzPlayerController::ThrowItem);
	InputComponent->BindAction("ActivateNextWeaponMode", EInputEvent::IE_Pressed, this, &AXyzPlayerController::ActivateNextWeaponMode);
	InputComponent->BindAction("UsePrimaryMeleeAttack", EInputEvent::IE_Pressed, this, &AXyzPlayerController::UsePrimaryMeleeAttack);
	InputComponent->BindAction("UseSecondaryMeleeAttack", EInputEvent::IE_Pressed, this, &AXyzPlayerController::UseSecondaryMeleeAttack);
	FInputActionBinding& ToggleMenuBinding = InputComponent->BindAction("ToggleMainMenu", EInputEvent::IE_Pressed, this, &AXyzPlayerController::ToggleMainMenu);
	ToggleMenuBinding.bExecuteWhenPaused = true;
	InputComponent->BindAction("InteractWithObject", EInputEvent::IE_Pressed, this, &AXyzPlayerController::InteractWithObject);
	InputComponent->BindAction("UseInventory", EInputEvent::IE_Pressed, this, &AXyzPlayerController::UseInventory);
	InputComponent->BindAction("UseRadialMenu", EInputEvent::IE_Pressed, this, &AXyzPlayerController::UseRadialMenu);
	InputComponent->BindAction("QuickSaveGame", EInputEvent::IE_Pressed, this, &AXyzPlayerController::QuickSaveGame);
	InputComponent->BindAction("QuickLoadGame", EInputEvent::IE_Pressed, this, &AXyzPlayerController::QuickLoadGame);
	InputComponent->BindAction("TogglePlayerMouseInput", EInputEvent::IE_Pressed, this, &AXyzPlayerController::TogglePlayerMouseInput);
	InputComponent->BindAction("QuitGame", EInputEvent::IE_Pressed, this, &AXyzPlayerController::QuitGame);

	InputComponent->BindAxis("TurnAtRate", this, &AXyzPlayerController::TurnAtRate);
	InputComponent->BindAxis("LookUpAtRate", this, &AXyzPlayerController::LookUpAtRate);
}

void AXyzPlayerController::CreateAndInitializeHUDWidgets()
{
	if (!IsValid(MainMenuWidget) && MainMenuWidgetClass.LoadSynchronous())
	{
		MainMenuWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass.LoadSynchronous());
	}

	if (!IsValid(PlayerHUDWidget) && PlayerHUDWidgetClass.LoadSynchronous())
	{
		PlayerHUDWidget = CreateWidget<UPlayerHUDWidget>(GetWorld(), PlayerHUDWidgetClass.LoadSynchronous());
	}

	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(PlayerHUDWidget) && IsValid(BaseCharacter))
	{
		UCharacterEquipmentComponent* CharacterEquipmentComponent = BaseCharacter->GetCharacterEquipmentComponent();
		UReticleWidget* ReticleWidget = PlayerHUDWidget->GetReticleWidget();
		if (IsValid(ReticleWidget))
		{
			BaseCharacter->OnAimingEvent.AddUObject(ReticleWidget, &UReticleWidget::OnAimingStateChanged);
			CharacterEquipmentComponent->OnEquipmentItemChangedEvent.AddUObject(ReticleWidget, &UReticleWidget::OnEquippedItemChanged);
		}

		UWeaponAmmoWidget* WeaponAmmoWidget = PlayerHUDWidget->GetWeaponAmmoWidget();
		if (IsValid(WeaponAmmoWidget))
		{
			CharacterEquipmentComponent->OnCurrentWeaponAmmoChangedEvent.AddUObject(WeaponAmmoWidget, &UWeaponAmmoWidget::OnWeaponAmmoChanged);
			CharacterEquipmentComponent->OnCurrentThrowableAmmoChangedEvent.AddUObject(WeaponAmmoWidget, &UWeaponAmmoWidget::OnThrowableAmmoChanged);
		}

		UCharacterAttributesWidget* CharacterAttributesWidget = PlayerHUDWidget->GetCharacterAttributesWidget();
		if (IsValid(CharacterAttributesWidget))
		{
			UXyzCharacterAttributeSet* AttributeSet = BaseCharacter->GetCharacterAttributes();
			AttributeSet->OnHealthChangedEvent.AddUObject(CharacterAttributesWidget, &UCharacterAttributesWidget::OnHealthChanged);
		}

		UCharacterAttributesWidget* CharacterAttributesCenterWidget = PlayerHUDWidget->GetCharacterAttributesCenterWidget();
		if (IsValid(CharacterAttributesCenterWidget))
		{
			UXyzCharacterAttributeSet* AttributeSet = BaseCharacter->GetCharacterAttributes();
			AttributeSet->OnStaminaChangedEvent.AddUObject(CharacterAttributesCenterWidget, &UCharacterAttributesWidget::OnStaminaChanged);
			AttributeSet->OnOxygenChangedEvent.AddUObject(CharacterAttributesCenterWidget, &UCharacterAttributesWidget::OnOxygenChanged);
		}

		PlayerHUDWidget->AddToViewport();
	}

	SetInputMode(FInputModeGameOnly{});
	bShowMouseCursor = false;
}

void AXyzPlayerController::RemoveHUDWidgets() const
{
	if (IsValid(MainMenuWidget))
	{
		MainMenuWidget->RemoveFromParent();
	}

	if (IsValid(PlayerHUDWidget))
	{
		AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
		if (IsValid(BaseCharacter))
		{
			UCharacterEquipmentComponent* CharacterEquipmentComponent = BaseCharacter->GetCharacterEquipmentComponent();
			const UReticleWidget* ReticleWidget = PlayerHUDWidget->GetReticleWidget();
			if (IsValid(ReticleWidget))
			{
				BaseCharacter->OnAimingEvent.RemoveAll(ReticleWidget);
				CharacterEquipmentComponent->OnEquipmentItemChangedEvent.RemoveAll(ReticleWidget);
			}

			const UWeaponAmmoWidget* WeaponAmmoWidget = PlayerHUDWidget->GetWeaponAmmoWidget();
			if (IsValid(WeaponAmmoWidget))
			{
				CharacterEquipmentComponent->OnCurrentWeaponAmmoChangedEvent.RemoveAll(WeaponAmmoWidget);
				CharacterEquipmentComponent->OnCurrentThrowableAmmoChangedEvent.RemoveAll(WeaponAmmoWidget);
			}

			const UCharacterAttributesWidget* CharacterAttributesWidget = PlayerHUDWidget->GetCharacterAttributesWidget();
			if (IsValid(CharacterAttributesWidget))
			{
				UXyzCharacterAttributeSet* AttributeSet = BaseCharacter->GetCharacterAttributes();
				AttributeSet->OnHealthChangedEvent.RemoveAll(CharacterAttributesWidget);
			}

			const UCharacterAttributesWidget* CharacterAttributesCenterWidget = PlayerHUDWidget->GetCharacterAttributesCenterWidget();
			if (IsValid(CharacterAttributesCenterWidget))
			{
				UXyzCharacterAttributeSet* AttributeSet = BaseCharacter->GetCharacterAttributes();
				AttributeSet->OnStaminaChangedEvent.RemoveAll(CharacterAttributesCenterWidget);
				AttributeSet->OnOxygenChangedEvent.RemoveAll(CharacterAttributesCenterWidget);
			}
		}

		PlayerHUDWidget->RemoveFromParent();
	}
}

void AXyzPlayerController::ToggleMainMenu()
{
	if (IsNetMode(NM_Standalone) || !IsValid(MainMenuWidget) || !IsValid(PlayerHUDWidget))
	{
		return;
	}

	if (MainMenuWidget->IsVisible())
	{
		MainMenuWidget->RemoveFromParent();
		PlayerHUDWidget->AddToViewport();
		SetInputMode(FInputModeGameOnly{});
		SetPause(false);
		bShowMouseCursor = false;
	}
	else
	{
		MainMenuWidget->AddToViewport();
		PlayerHUDWidget->RemoveFromParent();
		SetInputMode(FInputModeGameAndUI{});
		SetPause(true);
		bShowMouseCursor = true;
	}
}

void AXyzPlayerController::OnInteractableObjectFound(FName ActionName)
{
	if (!IsValid(PlayerInput) || !IsValid(PlayerHUDWidget))
	{
		return;
	}

	TArray<FInputActionKeyMapping> ActionKeys = PlayerInput->GetKeysForAction(ActionName);
	bool HasAnyKeys = ActionKeys.Num() != 0;
	if (HasAnyKeys)
	{
		FName ActionKey = ActionKeys[0].Key.GetFName();
		PlayerHUDWidget->SetInteractableKeyText(ActionKey);
	}
	PlayerHUDWidget->ShowInteractableKey(HasAnyKeys);
}

#pragma region INPUT ACTIONS

void AXyzPlayerController::MoveForward(float Value)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->MoveForward(Value);
	}
}

void AXyzPlayerController::MoveRight(float Value)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->MoveRight(Value);
	}
}

void AXyzPlayerController::Turn(float Value)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->Turn(Value);
	}
}

void AXyzPlayerController::LookUp(float Value)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->LookUp(Value);
	}
}

void AXyzPlayerController::Crouch()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->Crouch();
	}
}

void AXyzPlayerController::UnCrouch()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->UnCrouch();
	}
}

void AXyzPlayerController::Jump()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->StartJump();
	}
}

void AXyzPlayerController::StartSprint()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->StartSprint();
	}
}

void AXyzPlayerController::StopSprint()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->StopSprint();
	}
}

void AXyzPlayerController::ToggleProne()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->ToggleProne();
	}
}

void AXyzPlayerController::Mantle()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->StartMantle();
	}
}

void AXyzPlayerController::SwimForward(float Value)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->SwimForward(Value);
	}
}

void AXyzPlayerController::SwimRight(float Value)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->SwimRight(Value);
	}
}

void AXyzPlayerController::SwimUp(float Value)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->SwimUp(Value);
	}
}

void AXyzPlayerController::Dive()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->StartDive();
	}
}

void AXyzPlayerController::ClimbLadderUp(float Value)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->ClimbLadderUp(Value);
	}
}

void AXyzPlayerController::UseEnvironmentActor()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->ToggleUseEnvironmentActor();
	}
}

void AXyzPlayerController::JumpOffRunnableWall()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->JumpOffRunnableWall();
	}
}

void AXyzPlayerController::WallRun(float Value)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->WallRun(Value);
	}
}

void AXyzPlayerController::Slide()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->StartSlide();
	}
}

void AXyzPlayerController::ReloadLevel()
{
	if (IsNetMode(NM_Standalone))
	{
		const UWorld* World = GetWorld();
		FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(World);
		USaveSubsystem* SaveSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<USaveSubsystem>();
		SaveSubsystem->GetGameSaveDataMutable().bIsSerialized = false; // force disable loading saved data
		UGameplayStatics::OpenLevel(World, FName(*CurrentLevelName));
	}
}

void AXyzPlayerController::StartWeaponFire()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->StartWeaponFire();
	}
}

void AXyzPlayerController::StopFire()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->StopWeaponFire();
	}
}

void AXyzPlayerController::StartAim()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->StartAiming();
	}
}

void AXyzPlayerController::EndAim()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->StopAiming();
	}
}

void AXyzPlayerController::ReloadWeapon()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->StartWeaponReload();
	}
}

void AXyzPlayerController::DrawNextEquipmentItem()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->DrawNextItem();
	}
}

void AXyzPlayerController::DrawPreviousEquipmentItem()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->DrawPreviousItem();
	}
}

void AXyzPlayerController::TogglePrimaryItem()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->TogglePrimaryItem();
	}
}

void AXyzPlayerController::ThrowItem()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->StartItemThrow();
	}
}

void AXyzPlayerController::ActivateNextWeaponMode()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->ActivateNextWeaponMode();
	}
}

void AXyzPlayerController::UsePrimaryMeleeAttack()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->StartPrimaryMeleeAttack();
	}
}

void AXyzPlayerController::UseSecondaryMeleeAttack()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->StartSecondaryMeleeAttack();
	}
}

void AXyzPlayerController::InteractWithObject()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->InteractWithObject();
	}
}

void AXyzPlayerController::UseInventory()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->UseInventory(this);
	}
}

void AXyzPlayerController::UseRadialMenu()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->UseRadialMenu(this);
	}
}

void AXyzPlayerController::QuickSaveGame()
{
	if (IsNetMode(NM_Standalone))
	{
		USaveSubsystem* SaveSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<USaveSubsystem>();
		SaveSubsystem->SaveGame();
	}
}

void AXyzPlayerController::QuickLoadGame()
{
	if (IsNetMode(NM_Standalone))
	{
		USaveSubsystem* SaveSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<USaveSubsystem>();
		SaveSubsystem->LoadLastGame();
	}
}

void AXyzPlayerController::TogglePlayerMouseInput()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->TogglePlayerMouseInput(this);
	}
}

void AXyzPlayerController::QuitGame()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), this, EQuitPreference::Quit, false);
}
#pragma endregion

void AXyzPlayerController::TurnAtRate(float Value)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->TurnAtRate(Value);
	}
}

void AXyzPlayerController::LookUpAtRate(float Value)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->LookUpAtRate(Value);
	}
}
