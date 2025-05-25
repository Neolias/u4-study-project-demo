// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "Characters/XyzBaseCharacter.h"

#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "XyzHomeworkTypes.h"
#include "AbilitySystem/XyzAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/XyzCharacterAttributeSet.h"
#include "Actors/Environment/EnvironmentActor.h"
#include "Actors/Environment/Ladder.h"
#include "Actors/Environment/Zipline.h"
#include "Actors/Equipment/Throwables/ThrowableItem.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "Actors/Interactive/Interactable.h"
#include "Actors/Interactive/PickupItems/PickupItem.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"
#include "Controllers/XyzPlayerController.h"
#include "Curves/CurveVector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/Widgets/World/CharacterProgressBarWidget.h"

AXyzBaseCharacter::AXyzBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UXyzBaseCharMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	checkf(GetCharacterMovement()->IsA<UXyzBaseCharMovementComponent>(), TEXT("AXyzBaseCharacter::AXyzBaseCharacter(): can only be used with UXyzBaseCharMovementComponent."))
	AbilityActivationFlags.AddZeroed((uint32)EGameplayAbility::Max);
	InitializeGameplayAbilityCallbacks();
	BaseCharacterMovementComponent = StaticCast<UXyzBaseCharMovementComponent*>(GetCharacterMovement());
	CharacterEquipmentComponent = CreateDefaultSubobject<UCharacterEquipmentComponent>(TEXT("CharacterEquipment"));
	CharacterInventoryComponent = CreateDefaultSubobject<UCharacterInventoryComponent>(TEXT("CharacterInventory"));
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(GetCapsuleComponent());
	AbilitySystemComponent = CreateDefaultSubobject<UXyzAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AttributeSet = CreateDefaultSubobject<UXyzCharacterAttributeSet>(TEXT("AttributeSet"));
	GetMesh()->CastShadow = true;
	GetMesh()->bCastDynamicShadow = true;
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	BaseCharacterMovementComponent->CrouchedHalfHeight = 60.f;
	BaseCharacterMovementComponent->bCanWalkOffLedgesWhenCrouching = 1;
	BaseCharacterMovementComponent->Buoyancy = .9f;
	BaseCharacterMovementComponent->RotationRate = FRotator(540.f, 540.f, 540.f);
	BaseCharacterMovementComponent->bOrientRotationToMovement = 1;
	BaseCharacterMovementComponent->NavAgentProps.bCanCrouch = 1;

	ProneEyeHeight = BaseCharacterMovementComponent->GetProneCapsuleHalfHeight() * .8f;
	GetCapsuleComponent()->SetCapsuleHalfHeight(CollisionCapsuleHalfHeight);
	CrouchedEyeHeight = 32.f;
	CachedCollisionCapsuleScaledRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	CachedCollisionCapsuleScaledHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void AXyzBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	CachedPlayerController = Cast<AXyzPlayerController>(NewController);
	AAIController* AIController = Cast<AAIController>(NewController);
	if (IsValid(AIController))
	{
		FGenericTeamId TeamId = (uint8)Team;
		AIController->SetGenericTeamId(TeamId);
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(NewController, this);
	}
}

void AXyzBaseCharacter::UnPossessed()
{
	Super::UnPossessed();

	CachedPlayerController.Reset();
}

void AXyzBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	const UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *InventoryItemDataTable.GetUniqueID().GetAssetPathString());
	checkf(DataTable != nullptr, TEXT("AXyzBaseCharacter::BeginPlay(): InventoryItemDataTable in undefined."))

	AbilitySystemComponent->OnStartup();
	CharacterEquipmentComponent->SetInventoryItemDataTable(InventoryItemDataTable);
	CharacterInventoryComponent->SetInventoryItemDataTable(InventoryItemDataTable);

	OnReachedJumpApex.AddDynamic(this, &AXyzBaseCharacter::UpdateJumpApexHeight);
	LandedDelegate.AddDynamic(this, &AXyzBaseCharacter::OnCharacterLanded);
	AttributeSet->OnDeathEvent.AddUObject(this, &AXyzBaseCharacter::OnDeath);
	if (GetLocalRole() == ROLE_Authority)
	{
		GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AXyzBaseCharacter::OnCharacterCapsuleHit);
		OnTakeAnyDamage.AddDynamic(this, &AXyzBaseCharacter::OnDamageTaken);
		AttributeSet->OnOutOfStaminaEvent.AddUObject(this, &AXyzBaseCharacter::OnOutOfStamina);
		AttributeSet->OnOutOfOxygenEvent.AddUObject(this, &AXyzBaseCharacter::OnOutOfOxygen);

		CharacterEquipmentComponent->CreateLoadout();
	}

	SetupProgressBarWidget();
	EnableSignificance();
}

void AXyzBaseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateIKSettings(DeltaSeconds);
	LineTraceInteractiveObject();
	TryActivateGameplayAbilitiesWithFlags();
}

void AXyzBaseCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (OnInteractiveObjectFound.IsBound())
	{
		OnInteractiveObjectFound.Clear();
	}
}

FRotator AXyzBaseCharacter::GetAimOffset() const
{
	FVector AimDirectionWorld = GetBaseAimRotation().Vector();
	FVector AimDirectionLocal = GetTransform().InverseTransformVector(AimDirectionWorld);
	return AimDirectionLocal.ToOrientationRotator();
}

bool AXyzBaseCharacter::IsAnimMontagePlaying() const
{
	return GetMesh()->GetAnimInstance() && GetMesh()->GetAnimInstance()->IsAnyMontagePlaying();
}

void AXyzBaseCharacter::EnableRagdoll() const
{
	BaseCharacterMovementComponent->DisableMovement();
	GetCapsuleComponent()->SetCollisionProfileName("NoCollision");
	GetMesh()->SetCollisionProfileName(CollisionProfileRagdoll);
	GetMesh()->SetSimulatePhysics(true);
}

//@ IGenericTeamAgentInterface
FGenericTeamId AXyzBaseCharacter::GetGenericTeamId() const
{
	return FGenericTeamId((uint8)Team);
}

//~ IGenericTeamAgentInterface

//@ SaveSubsystemInterface
void AXyzBaseCharacter::OnLevelDeserialized_Implementation() {}
//~ SaveSubsystemInterface

void AXyzBaseCharacter::RecalculateBaseEyeHeight()
{
	if (IsProne() || IsSliding())
	{
		BaseEyeHeight = ProneEyeHeight;
	}
	else if (IsCrouching())
	{
		BaseEyeHeight = CrouchedEyeHeight;
	}
	else
	{
		BaseEyeHeight = CrouchedEyeHeight;
		Super::RecalculateBaseEyeHeight();
	}
}

void AXyzBaseCharacter::SetupProgressBarWidget()
{
	const UCharacterProgressBarWidget* ProgressBarWidget = Cast<UCharacterProgressBarWidget>(WidgetComponent->GetUserWidgetObject());
	if (!ProgressBarWidget)
	{
		WidgetComponent->SetVisibility(false);
		return;
	}

	if (IsLocallyControlled() && Controller->IsA<APlayerController>())
	{
		WidgetComponent->SetVisibility(false);
	}

	AttributeSet->OnHealthChangedEvent.AddUObject(ProgressBarWidget, &UCharacterProgressBarWidget::SetHealthProgressBar);
	AttributeSet->OnDeathEvent.AddLambda([=](bool bShouldPlayAnimation) { WidgetComponent->SetVisibility(false); });
	ProgressBarWidget->SetHealthProgressBar(AttributeSet->GetHealth());
}

#pragma region SIGNIFICANCE MANAGER

void AXyzBaseCharacter::EnableSignificance()
{
	if (bIsSignificanceEnabled && GetLocalRole() != ROLE_Authority && !IsLocallyControlled())
	{
		USignificanceManager* SignificanceManager = FSignificanceManagerModule::Get(GetWorld());
		if (SignificanceManager)
		{
			SignificanceManager->RegisterObject(
				this,
				SignificanceTagCharacter,
				[this](USignificanceManager::FManagedObjectInfo* ObjectInfo, const FTransform& ViewPoint) -> float
				{
					return SignificanceFunction(ObjectInfo, ViewPoint);
				},
				USignificanceManager::EPostSignificanceType::Sequential,
				[this](USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSignificance, float Significance,
				       bool bFinal)
				{
					PostSignificanceFunction(ObjectInfo, OldSignificance, Significance, bFinal);
				}
			);
		}
	}
}

float AXyzBaseCharacter::SignificanceFunction(USignificanceManager::FManagedObjectInfo* ObjectInfo, FTransform ViewPoint)
{
	if (ObjectInfo && ObjectInfo->GetTag() == SignificanceTagCharacter)
	{
		AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ObjectInfo->GetObject());
		if (!IsValid(BaseCharacter))
		{
			return SignificanceValueVeryHigh;
		}

		float DistSquared = FVector::DistSquared(BaseCharacter->GetActorLocation(), ViewPoint.GetLocation());
		if (DistSquared <= FMath::Square(VeryHighSignificanceDistance))
		{
			return SignificanceValueVeryHigh;
		}
		if (DistSquared <= FMath::Square(HighSignificanceDistance))
		{
			return SignificanceValueHigh;
		}
		if (DistSquared <= FMath::Square(MediumSignificanceDistance))
		{
			return SignificanceValueMedium;
		}
		if (DistSquared <= FMath::Square(LowSignificanceDistance))
		{
			return SignificanceValueLow;
		}
		return SignificanceValueVeryLow;
	}
	return SignificanceValueVeryHigh;
}

void AXyzBaseCharacter::PostSignificanceFunction(USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSignificance, float Significance, bool bFinal)
{
	if (OldSignificance == Significance || ObjectInfo->GetTag() != SignificanceTagCharacter)
	{
		return;
	}

	AXyzBaseCharacter* Character = StaticCast<AXyzBaseCharacter*>(ObjectInfo->GetObject());
	if (!IsValid(Character))
	{
		return;
	}

	if (Significance == SignificanceValueVeryHigh)
	{
		SetSignificanceSettings(Character, 0.f, true, .0f, true, 0.f);
	}
	else if (Significance == SignificanceValueHigh)
	{
		SetSignificanceSettings(Character, .01f, true, .01f, true, .01f);
	}
	else if (Significance == SignificanceValueMedium)
	{
		SetSignificanceSettings(Character, .05f, false, .05f, true, .05f);
	}
	else if (Significance == SignificanceValueLow)
	{
		SetSignificanceSettings(Character, .5f, false, .5f, true, .5f);
	}
	else if (Significance == SignificanceValueVeryLow)
	{
		SetSignificanceSettings(Character, 5.f, false, 5.f, false);
	}
}

void AXyzBaseCharacter::SetSignificanceSettings(AXyzBaseCharacter* Character, float MovementComponentTickInterval, bool bWidgetComponentVisibility, float AIControllerTickInterval, bool bMeshTickEnabled, float MeshTickInterval/* = 0.f*/)
{
	if (!IsValid(Character))
	{
		return;
	}
	
	Character->GetCharacterMovement()->SetComponentTickInterval(MovementComponentTickInterval);

	if (UWidgetComponent* Widget = Character->GetCharacterWidgetComponent())
	{
		Widget->SetVisibility(!Character->IsDead() && bWidgetComponentVisibility);
	}

	AAIController* AIController = Character->GetController<AAIController>();
	if (IsValid(AIController))
	{
		AIController->SetActorTickInterval(AIControllerTickInterval);
	}
	
	Character->GetMesh()->SetComponentTickEnabled(bMeshTickEnabled);
	if (bMeshTickEnabled)
	{
		Character->GetMesh()->SetComponentTickInterval(MeshTickInterval);
	}
}
#pragma endregion

#pragma region GAMEPLAY ABILITY SYSTEM

void AXyzBaseCharacter::SetGameplayAbilityActivationFlag(EGameplayAbility Ability, EGameplayAbilityActivationFlag ActivationFlag)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	EGameplayAbilityActivationFlag CurrentFlag = AbilityActivationFlags[(uint32)Ability];
	if (CurrentFlag == ActivationFlag)
	{
		return;
	}

	bool bIsActivated = false;
	bool bCheckActivation = false;
	bool bSetNewFlag = false;
	switch (ActivationFlag)
	{
		case EGameplayAbilityActivationFlag::TryActivateOnce:
			bIsActivated = true;
			break;
		case EGameplayAbilityActivationFlag::TryCancelOnce:
			break;
		case EGameplayAbilityActivationFlag::TryActivateRepeat:
			bIsActivated = true;
			bCheckActivation = true;
			break;
		case EGameplayAbilityActivationFlag::TryCancelRepeat:
			bCheckActivation = true;
			break;
		case EGameplayAbilityActivationFlag::ActivateRepeat:
			bIsActivated = true;
			bSetNewFlag = true;
			break;
		case EGameplayAbilityActivationFlag::CancelRepeat:
			bSetNewFlag = true;
			break;
		default:
			return;
	}

	// Trying to execute activation immediately
	AbilitySystemComponent->ActivateAbility(Ability, bIsActivated);
	if (bCheckActivation)
	{
		bSetNewFlag = AbilitySystemComponent->IsAbilityActiveRemote(Ability) != bIsActivated;
	}
	if (!bSetNewFlag)
	{
		ActivationFlag = EGameplayAbilityActivationFlag::None;
	}
	// Pass activation to the tick function if activation failed or is set to repeat
	AbilityActivationFlags[(uint32)Ability] = ActivationFlag;
}

//@ IAbilitySystemInterface
UAbilitySystemComponent* AXyzBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

//~ IAbilitySystemInterface

void AXyzBaseCharacter::InitializeGameplayAbilityCallbacks()
{
	AbilityActivationCallbacks.AddDefaulted((uint32)EGameplayAbility::Max);
	AbilityDeactivationCallbacks.AddDefaulted((uint32)EGameplayAbility::Max);

	AbilityActivationCallbacks[(uint32)EGameplayAbility::Sprint] = [this] { StartSprintInternal(); };
	AbilityDeactivationCallbacks[(uint32)EGameplayAbility::Sprint] = [this] { StopSprintInternal(); };
	AbilityActivationCallbacks[(uint32)EGameplayAbility::Crouch] = [this] { CrouchInternal(); };
	AbilityDeactivationCallbacks[(uint32)EGameplayAbility::Crouch] = [this] { UnCrouchInternal(); };
	AbilityActivationCallbacks[(uint32)EGameplayAbility::Prone] = [this] { ProneInternal(); };
	AbilityDeactivationCallbacks[(uint32)EGameplayAbility::Prone] = [this] { UnProneInternal(); };
	AbilityActivationCallbacks[(uint32)EGameplayAbility::Slide] = [this] { StartSlideInternal(); };
	AbilityDeactivationCallbacks[(uint32)EGameplayAbility::Slide] = [this] { StopSlideInternal(); };
	AbilityActivationCallbacks[(uint32)EGameplayAbility::Jump] = [this] { StartJumpInternal(); };
	AbilityActivationCallbacks[(uint32)EGameplayAbility::Mantle] = [this] { StartMantleInternal(); };
	AbilityActivationCallbacks[(uint32)EGameplayAbility::UseEnvironmentActor] = [this] { StartUseEnvironmentActorInternal(); };
	AbilityDeactivationCallbacks[(uint32)EGameplayAbility::UseEnvironmentActor] = [this] { StopUseEnvironmentActorInternal(); };
	AbilityActivationCallbacks[(uint32)EGameplayAbility::Dive] = [this] { StartDiveInternal(); };
	AbilityDeactivationCallbacks[(uint32)EGameplayAbility::Dive] = [this] { StopDiveInternal(); };
	// AbilityActivationCallbacks[(uint32)EGameplayAbility::Death] = [this] { ; }; // server-only logic inside the ability class
	AbilityActivationCallbacks[(uint32)EGameplayAbility::OutOfStamina] = [this] { StartOutOfStaminaInternal(); };
	AbilityDeactivationCallbacks[(uint32)EGameplayAbility::OutOfStamina] = [this] { StopOutOfStaminaInternal(); };
	// AbilityActivationCallbacks[(uint32)EGameplayAbility::DrawNextItem] = [this] { ; }; // server-only logic inside the ability class
	// AbilityActivationCallbacks[(uint32)EGameplayAbility::DrawPreviousItem] = [this] { ; }; // server-only logic inside the ability class
	AbilityActivationCallbacks[(uint32)EGameplayAbility::DrawPrimaryItem] = [this] { DrawPrimaryItemInternal(); };
	AbilityDeactivationCallbacks[(uint32)EGameplayAbility::DrawPrimaryItem] = [this] { SheathePrimaryItemInternal(); };
	// AbilityActivationCallbacks[(uint32)EGameplayAbility::EquipFromCurrentSlot] = [this] { ; }; // server-only logic inside the ability class
	// AbilityActivationCallbacks[(uint32)EGameplayAbility::EquipFromCurrentSlotFast] = [this] { ; }; // server-only logic inside the ability class
	AbilityActivationCallbacks[(uint32)EGameplayAbility::AimWeapon] = [this] { StartAimingInternal(); };
	AbilityDeactivationCallbacks[(uint32)EGameplayAbility::AimWeapon] = [this] { StopAimingInternal(); };
	AbilityActivationCallbacks[(uint32)EGameplayAbility::FireWeapon] = [this] { StartWeaponFireInternal(); };
	AbilityDeactivationCallbacks[(uint32)EGameplayAbility::FireWeapon] = [this] { StopWeaponFireInternal(); };
	AbilityActivationCallbacks[(uint32)EGameplayAbility::ReloadWeapon] = [this] { StartWeaponReloadInternal(); };
	AbilityDeactivationCallbacks[(uint32)EGameplayAbility::ReloadWeapon] = [this] { StopWeaponReloadInternal(); };
	AbilityActivationCallbacks[(uint32)EGameplayAbility::PrimaryMeleeAttack] = [this] { StartPrimaryMeleeAttackInternal(); };
	AbilityDeactivationCallbacks[(uint32)EGameplayAbility::PrimaryMeleeAttack] = [this] { StopPrimaryMeleeAttackInternal(); };
	AbilityActivationCallbacks[(uint32)EGameplayAbility::SecondaryMeleeAttack] = [this] { StartSecondaryMeleeAttackInternal(); };
	AbilityDeactivationCallbacks[(uint32)EGameplayAbility::SecondaryMeleeAttack] = [this] { StopSecondaryMeleeAttackInternal(); };
	AbilityActivationCallbacks[(uint32)EGameplayAbility::ThrowItem] = [this] { StartItemThrowInternal(); };
	AbilityDeactivationCallbacks[(uint32)EGameplayAbility::ThrowItem] = [this] { StopItemThrowInternal(); };
}

void AXyzBaseCharacter::TryActivateGameplayAbilitiesWithFlags()
{
	for (int32 i = 1; i < (int32)EGameplayAbility::Max; ++i)
	{
		EGameplayAbilityActivationFlag ActivationFlag = AbilityActivationFlags[i];
		EGameplayAbility Ability = (EGameplayAbility)i;
		switch (ActivationFlag)
		{
			case EGameplayAbilityActivationFlag::TryActivateRepeat:
				AbilitySystemComponent->ActivateAbility(Ability, true);
				if (AbilitySystemComponent->IsAbilityActiveRemote(Ability))
				{
					AbilityActivationFlags[i] = EGameplayAbilityActivationFlag::None;
				}
				break;
			case EGameplayAbilityActivationFlag::TryCancelRepeat:
				AbilitySystemComponent->ActivateAbility(Ability, false);
				if (!AbilitySystemComponent->IsAbilityActiveRemote(Ability))
				{
					AbilityActivationFlags[i] = EGameplayAbilityActivationFlag::None;
				}
				break;
			case EGameplayAbilityActivationFlag::ActivateRepeat:
				AbilitySystemComponent->ActivateAbility(Ability, true);
				break;
			case EGameplayAbilityActivationFlag::CancelRepeat:
				AbilitySystemComponent->ActivateAbility(Ability, false);
				break;
			default:
				break;
		}
	}
}

void AXyzBaseCharacter::ExecuteGameplayAbilityCallbackInternal(EGameplayAbility Ability, bool bIsActivationCallback, bool bIsServerCall/* = true*/)
{
	// Discarding the ability activation if it is already activated on AutonomousProxy
	// Used to avoid duplicated activation of predicted abilities
	if (bIsServerCall && GetLocalRole() == ROLE_AutonomousProxy && AbilitySystemComponent->IsAbilityActiveLocal(Ability) == bIsActivationCallback)
	{
		return;
	}

	uint32 CallbackIndex = (uint32)Ability;
	TFunction<void()> AbilityCallback;
	if (bIsActivationCallback)
	{
		AbilityCallback = AbilityActivationCallbacks[CallbackIndex];
	}
	else
	{
		AbilityCallback = AbilityDeactivationCallbacks[CallbackIndex];
	}

	if (AbilityCallback)
	{
		AbilityCallback();
	}
}

void AXyzBaseCharacter::Multicast_ExecuteGameplayAbilityCallback_Implementation(EGameplayAbility Ability, bool bIsActivationCallback)
{
	ExecuteGameplayAbilityCallbackInternal(Ability, bIsActivationCallback);
}

#pragma endregion

#pragma region INVENTORY

void AXyzBaseCharacter::TogglePlayerMouseInput(APlayerController* PlayerController)
{
	if (!IsValid(PlayerController))
	{
		return;
	}

	bool bIsMouseInputEnabled = PlayerController->bShowMouseCursor;
	PlayerController->bShowMouseCursor = !bIsMouseInputEnabled;
	if (bIsMouseInputEnabled)
	{
		PlayerController->SetInputMode(FInputModeGameOnly{});
	}
	else
	{
		PlayerController->SetInputMode(FInputModeGameAndUI{});
	}
}

void AXyzBaseCharacter::UpdatePlayerMouseInput(APlayerController* PlayerController) const
{
	if (!IsValid(PlayerController))
	{
		return;
	}

	if (CharacterInventoryComponent->IsViewInventoryVisible() || CharacterEquipmentComponent->IsViewEquipmentVisible() || CharacterEquipmentComponent->IsRadialMenuVisible())
	{
		PlayerController->SetInputMode(FInputModeGameAndUI{});
		PlayerController->bShowMouseCursor = true;
	}
	else
	{
		PlayerController->SetInputMode(FInputModeGameOnly{});
		PlayerController->bShowMouseCursor = false;
	}
}

void AXyzBaseCharacter::UseInventory(APlayerController* PlayerController) const
{
	if (!IsValid(PlayerController))
	{
		return;
	}

	if (!CharacterInventoryComponent->IsViewInventoryVisible() || !CharacterEquipmentComponent->IsViewEquipmentVisible())
	{
		CharacterInventoryComponent->OpenViewInventory(PlayerController);
		CharacterEquipmentComponent->OpenViewEquipment(PlayerController);
	}
	else
	{
		CharacterInventoryComponent->CloseViewInventory();
		CharacterEquipmentComponent->CloseViewEquipment();
	}

	UpdatePlayerMouseInput(PlayerController);
}

void AXyzBaseCharacter::UseRadialMenu(APlayerController* PlayerController) const
{
	if (!IsValid(PlayerController))
	{
		return;
	}

	if (!CharacterEquipmentComponent->IsRadialMenuVisible())
	{
		CharacterEquipmentComponent->OpenRadialMenu(PlayerController);
	}
	else
	{
		CharacterEquipmentComponent->CloseRadialMenu();
	}

	UpdatePlayerMouseInput(PlayerController);
}

bool AXyzBaseCharacter::PickupItem(EInventoryItemType ItemType, int32 Amount) const
{
	return CharacterInventoryComponent->AddInventoryItem(ItemType, Amount);
}

void AXyzBaseCharacter::DropItem(EInventoryItemType ItemType, int32 Amount) const
{
	if (const UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *InventoryItemDataTable.GetUniqueID().GetAssetPathString()))
	{
		FString RowID = UEnum::GetDisplayValueAsText<EInventoryItemType>(ItemType).ToString();
		const FInventoryTableRow* ItemData = DataTable->FindRow<FInventoryTableRow>(FName(RowID), TEXT("Find item data"));
		if (ItemData && ItemData->InventoryItemDescription.PickUpItemClass.LoadSynchronous())
		{
			FTransform SpawnTransform = GetActorTransform();
			float SpawnRadius = GetCapsuleComponent()->GetScaledCapsuleRadius() * 3;
			float PosX = FMath::RandRange(-SpawnRadius, SpawnRadius);
			float PosY = FMath::RandRange(-SpawnRadius, SpawnRadius);
			SpawnTransform.AddToTranslation(FVector(PosX, PosY, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
			float Yaw = FMath::RandRange(-180.f, 180.f);
			SpawnTransform.SetRotation(FRotator(0.f, Yaw, 0.f).Quaternion());
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			APickupItem* PickupItem = GetWorld()->SpawnActor<APickupItem>(ItemData->InventoryItemDescription.PickUpItemClass.LoadSynchronous(), SpawnTransform, SpawnParameters);

			if (IsValid(PickupItem))
			{
				PickupItem->SetAmount(Amount);
			}
		}
	}
}

void AXyzBaseCharacter::Server_UseItem_Implementation(UInventoryItem* InventoryItem, UInventorySlot* InventorySlot/* = nullptr*/)
{
	if (InventoryItem && InventoryItem->IsEquipment() && InventoryItem->GetEquipmentItemClass().LoadSynchronous())
	{
		if (CharacterEquipmentComponent->AddEquipmentItemByClass(InventoryItem->GetEquipmentItemClass().LoadSynchronous(), InventoryItem->GetCount()))
		{
			if (IsValid(InventorySlot))
			{
				CharacterInventoryComponent->RemoveInventoryItemBySlot(InventorySlot, InventoryItem->GetCount());
			}
			else
			{
				CharacterInventoryComponent->RemoveInventoryItemByType(InventoryItem->GetInventoryItemType(), InventoryItem->GetCount());
			}
		}
	}
	else
	{
		InventoryItem->Consume(this);
	}
}

void AXyzBaseCharacter::Server_AddEquipmentItem_Implementation(EInventoryItemType ItemType, int32 Amount/* = 1*/, int32 EquipmentSlotIndex/* = -1*/)
{
	if (!CharacterEquipmentComponent->AddEquipmentItemByType(ItemType, Amount, EquipmentSlotIndex) && !CharacterInventoryComponent->AddInventoryItem(ItemType, Amount))
	{
		DropItem(ItemType, Amount);
	}
}

void AXyzBaseCharacter::Server_RemoveEquipmentItem_Implementation(int32 EquipmentSlotIndex)
{
	const AEquipmentItem* EquippedItem = CharacterEquipmentComponent->GetEquipmentItemInSlot(EquipmentSlotIndex);
	if (IsValid(EquippedItem) && IsValid(EquippedItem->GetLinkedInventoryItem()) && CharacterEquipmentComponent->RemoveEquipmentItem(EquipmentSlotIndex))
	{
		const UInventoryItem* InventoryItem = EquippedItem->GetLinkedInventoryItem();
		if (!PickupItem(InventoryItem->GetInventoryItemType(), InventoryItem->GetCount()))
		{
			DropItem(InventoryItem->GetInventoryItemType(), InventoryItem->GetCount());
		}
	}
}

void AXyzBaseCharacter::Server_DropItem_Implementation(EInventoryItemType ItemType, int32 Amount)
{
	DropItem(ItemType, Amount);
}

bool AXyzBaseCharacter::AddAmmoToInventory(EWeaponAmmoType AmmoType, int32 Amount) const
{
	return CharacterInventoryComponent->AddAmmoItem(AmmoType, Amount);
}

int32 AXyzBaseCharacter::RemoveAmmoFromInventory(EWeaponAmmoType AmmoType, int32 Amount) const
{
	return CharacterInventoryComponent->RemoveAmmoItem(AmmoType, Amount);
}
#pragma endregion

#pragma region INTERACTIVE OBJECTS

void AXyzBaseCharacter::InteractWithObject()
{
	if (CurrentInteractableObject.GetInterface())
	{
		Server_InteractWithObject(CurrentInteractableObject.GetObject());
	}
}

void AXyzBaseCharacter::Server_InteractWithObject_Implementation(UObject* InteractableObject)
{
	Multicast_InteractWithObject(InteractableObject);
}

void AXyzBaseCharacter::Multicast_InteractWithObject_Implementation(UObject* InteractableObject)
{
	CurrentInteractableObject = InteractableObject;
	if (CurrentInteractableObject.GetInterface())
	{
		CurrentInteractableObject->Interact(this);
	}
}

void AXyzBaseCharacter::LineTraceInteractiveObject()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	FVector ViewPointLocation;
	FRotator ViewPointRotation;
	GetController()->GetPlayerViewPoint(ViewPointLocation, ViewPointRotation);

	FHitResult HitResult;
	FVector EndLocation = ViewPointLocation + ViewPointRotation.Vector() * InteractiveObjectDetectionRange;
	GetWorld()->LineTraceSingleByChannel(HitResult, ViewPointLocation, EndLocation, ECC_Visibility);

	if (CurrentInteractableObject != HitResult.GetActor())
	{
		CurrentInteractableObject = HitResult.GetActor();
		if (OnInteractiveObjectFound.IsBound())
		{
			if (CurrentInteractableObject.GetInterface())
			{
				OnInteractiveObjectFound.Broadcast(CurrentInteractableObject->GetActionName());
			}
			else
			{
				OnInteractiveObjectFound.Broadcast(NAME_None);
			}
		}
	}
}
#pragma endregion

#pragma region INVERSE KINEMATICS

float AXyzBaseCharacter::GetIKLeftFootOffsetZ() const
{
	return IKLeftFootOffset;
}

float AXyzBaseCharacter::GetIKRightFootOffsetZ() const
{
	return IKRightFootOffset;
}

float AXyzBaseCharacter::GetIKPelvisOffsetZ() const
{
	return IKPelvisOffset;
}

void AXyzBaseCharacter::UpdateIKSettings(float DeltaSeconds)
{
	IKLeftFootOffset = FMath::FInterpTo(IKLeftFootOffset, GetIKOffsetForFootSocketZ(LeftFootSocketName), DeltaSeconds, IKInterpSpeed);
	IKRightFootOffset = FMath::FInterpTo(IKRightFootOffset, GetIKOffsetForFootSocketZ(RightFootSocketName), DeltaSeconds, IKInterpSpeed);
	IKPelvisOffset = FMath::FInterpTo(IKPelvisOffset, GetIKOffsetForPelvisSocketZ(), DeltaSeconds, IKInterpSpeed);
}

float AXyzBaseCharacter::GetIKOffsetForFootSocketZ(FName SocketName) const
{
	FVector SocketLocation = GetMesh()->GetSocketLocation(SocketName);
	FVector TraceStart(SocketLocation.X, SocketLocation.Y, GetActorLocation().Z);
	FVector CharacterBottomLocation = GetActorLocation();
	CharacterBottomLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector TraceEnd(SocketLocation.X, SocketLocation.Y, CharacterBottomLocation.Z - MaxFeetIKOffsetZ);

	FHitResult HitResult;
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	FVector FootBoxSize(10.f, 1.f, 5.f);
	if (UKismetSystemLibrary::BoxTraceSingle(GetWorld(), TraceStart, TraceEnd, FootBoxSize, GetMesh()->GetSocketRotation(SocketName), TraceType, true, TArray<AActor*>(), EDrawDebugTrace::None, HitResult, true))
	{
		return FMath::Clamp(HitResult.Location.Z - CharacterBottomLocation.Z, -MaxFeetIKOffsetZ, 0.f);
	}

	return 0.f;
}

float AXyzBaseCharacter::GetIKOffsetForPelvisSocketZ() const
{
	return FMath::Min(IKLeftFootOffset, IKRightFootOffset);
}
#pragma endregion

#pragma region MOVEMENT

void AXyzBaseCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (BaseCharacterMovementComponent->IsMovingOnGround())
	{
		if (PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_WallRun)
		{
			// Delay equipping weapons when jumping between walls
			GetWorld()->GetTimerManager().SetTimer(WallRunEquipItemTimerHandle, [this] { EquipItemFromCurrentSlot(); }, WallRunEquipItemTimerLength, false);
		}
		else if (PrevMovementMode != MOVE_Falling)
		{
			EquipItemFromCurrentSlot();
		}
	}
	else
	{
		bool bWasFiringWeapon = IsFiringWeapon();
		GetWorld()->GetTimerManager().ClearTimer(WallRunEquipItemTimerHandle);
		CharacterEquipmentComponent->UnequipCurrentItem();

		if (BaseCharacterMovementComponent->IsFalling())
		{
			// Cancel aiming and request it again
			if (IsAiming())
			{
				StopAiming();
				if (bWasFiringWeapon)
				{
					StartWeaponFire();
				}
				StartAiming();
			}

			if (IsCrouching())
			{
				UnCrouch();
			}
		}
		else if (BaseCharacterMovementComponent->IsSwimming())
		{
			if (GetLocalRole() == ROLE_Authority)
			{
				if (IsCrouching())
				{
					// Initiating UnProne and UnCrouch from server to update the capsule size correctly on clients
					AbilityActivationFlags[(uint32)EGameplayAbility::Prone] = EGameplayAbilityActivationFlag::TryCancelRepeat;
					AbilityActivationFlags[(uint32)EGameplayAbility::Crouch] = EGameplayAbilityActivationFlag::TryCancelRepeat;
				}
				else if (!IsSliding())
				{
					BaseCharacterMovementComponent->Multicast_UpdateSwimmingCapsuleSize();
				}
			}
			bUseControllerRotationPitch = true;
		}
	}

	if (PrevMovementMode == MOVE_Swimming)
	{
		BaseCharacterMovementComponent->UpdateSwimmingCapsuleSize();
		bUseControllerRotationPitch = false;
	}
}

void AXyzBaseCharacter::StartJump()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::Jump, EGameplayAbilityActivationFlag::TryActivateOnce);
}

void AXyzBaseCharacter::StartJumpInternal()
{
	Jump();
}
#pragma endregion

#pragma region SWIMMING

bool AXyzBaseCharacter::IsDiveAbilityActive() const
{
	return AbilitySystemComponent->IsAbilityActiveRemote(EGameplayAbility::Dive);
}

void AXyzBaseCharacter::StartDive()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::Dive, EGameplayAbilityActivationFlag::TryActivateOnce);
}

void AXyzBaseCharacter::StopDive()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::Dive, EGameplayAbilityActivationFlag::TryCancelOnce);
}

bool AXyzBaseCharacter::IsDiving() const
{
	return AbilitySystemComponent->IsEffectActiveRemote(EGameplayEffect::Diving);
}

void AXyzBaseCharacter::OnDiving(bool bIsDiving) const
{
	AbilitySystemComponent->ApplyEffectToSelf(EGameplayEffect::Diving, bIsDiving);
}

void AXyzBaseCharacter::StartDiveInternal()
{
	const auto TimerCallback = FTimerDelegate::CreateLambda([this]
	{
		BaseCharacterMovementComponent->SetMovementFlag((uint32)EGameplayAbility::Dive, true);
		StopDive();
	});
	GetWorld()->GetTimerManager().SetTimer(DiveTimerHandle, TimerCallback, DiveAbilityLength, false);
}

void AXyzBaseCharacter::StopDiveInternal() const
{
	BaseCharacterMovementComponent->SetMovementFlag((uint32)EGameplayAbility::Dive, false);
	BaseCharacterMovementComponent->OnDiving(true);
}
#pragma endregion

#pragma region SPRINTING

bool AXyzBaseCharacter::IsSprinting() const
{
	return AbilitySystemComponent->IsAbilityActiveRemote(EGameplayAbility::Sprint);
}

void AXyzBaseCharacter::StartSprint()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::Sprint, EGameplayAbilityActivationFlag::ActivateRepeat);
}

void AXyzBaseCharacter::StopSprint()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::Sprint, EGameplayAbilityActivationFlag::TryCancelRepeat);
}

void AXyzBaseCharacter::StartSprintInternal()
{
	if (IsCrouching())
	{
		if (IsProne())
		{
			UnProne();
		}
		UnCrouch();
	}
	BaseCharacterMovementComponent->StartSprint();
}

void AXyzBaseCharacter::StopSprintInternal() const
{
	BaseCharacterMovementComponent->StopSprint();
}

void AXyzBaseCharacter::OnStartSprint_Implementation()
{
	CharacterEquipmentComponent->UnequipCurrentItem();
}

void AXyzBaseCharacter::OnStopSprint_Implementation()
{
	EquipItemFromCurrentSlot(true);
}
#pragma endregion

#pragma region SLIDING

bool AXyzBaseCharacter::IsSliding() const
{
	return AbilitySystemComponent->IsAbilityActiveRemote(EGameplayAbility::Slide);
}

void AXyzBaseCharacter::StartSlide()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::Slide, EGameplayAbilityActivationFlag::TryActivateOnce);
}

void AXyzBaseCharacter::StopSlide()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::Slide, EGameplayAbilityActivationFlag::TryCancelOnce);
}

void AXyzBaseCharacter::OnStartSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	RecalculateBaseEyeHeight();

	if (HalfHeightAdjust != 0.f && ScaledHalfHeightAdjust != 0.f)
	{
		const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
		FVector MeshRelativeLocation = GetMesh()->GetRelativeLocation();
		MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z + HalfHeightAdjust;
		GetMesh()->SetRelativeLocation(MeshRelativeLocation);
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}

	AController* CharacterController = GetController();
	if (IsValid(CharacterController))
	{
		CharacterController->SetIgnoreLookInput(true);
		CharacterController->SetIgnoreMoveInput(true);
	}

	CharacterEquipmentComponent->UnequipCurrentItem();

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SlideAnimMontage)
	{
		if (AnimInstance->Montage_IsPlaying(SlideAnimMontage))
		{
			return;
		}
		AnimInstance->Montage_Play(SlideAnimMontage);
		FOnMontageEnded OnMontageEndedDelegate;
		OnMontageEndedDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
		{
			StopSlide();
		});
		AnimInstance->Montage_SetEndDelegate(OnMontageEndedDelegate, SlideAnimMontage);
	}
	else
	{
		StopSlide();
	}
}

void AXyzBaseCharacter::OnStopSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	RecalculateBaseEyeHeight();

	if (HalfHeightAdjust != 0.f && ScaledHalfHeightAdjust != 0.f)
	{
		const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
		FVector MeshRelativeLocation = GetMesh()->GetRelativeLocation();
		MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z;
		GetMesh()->SetRelativeLocation(MeshRelativeLocation);
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}

	AController* CharacterController = GetController();
	if (IsValid(CharacterController))
	{
		CharacterController->ResetIgnoreLookInput();
		CharacterController->ResetIgnoreMoveInput();
	}

	EquipItemFromCurrentSlot();
}

void AXyzBaseCharacter::StartSlideInternal() const
{
	BaseCharacterMovementComponent->StartSlide();
}

void AXyzBaseCharacter::StopSlideInternal()
{
	BaseCharacterMovementComponent->StopSlide();
	if (BaseCharacterMovementComponent->IsSwimming())
	{
		StopSprint();
		BaseCharacterMovementComponent->UpdateSwimmingCapsuleSize();
	}
	else if (!BaseCharacterMovementComponent->CanUnCrouch())
	{
		StopSprint();
		SetGameplayAbilityActivationFlag(EGameplayAbility::Crouch, EGameplayAbilityActivationFlag::TryActivateRepeat);
	}
}
#pragma endregion

#pragma region CROUCHING

bool AXyzBaseCharacter::IsCrouching() const
{
	return AbilitySystemComponent->IsAbilityActiveRemote(EGameplayAbility::Crouch);
}

void AXyzBaseCharacter::Crouch(bool bClientSimulation)
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::Crouch, EGameplayAbilityActivationFlag::TryActivateOnce);
}

void AXyzBaseCharacter::UnCrouch(bool bClientSimulation)
{
	UnProne();
	SetGameplayAbilityActivationFlag(EGameplayAbility::Crouch, EGameplayAbilityActivationFlag::TryCancelRepeat);
}

void AXyzBaseCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	RecalculateBaseEyeHeight();

	if (HalfHeightAdjust != 0.f && ScaledHalfHeightAdjust != 0.f)
	{
		const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
		FVector MeshRelativeLocation = GetMesh()->GetRelativeLocation();
		MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z + HalfHeightAdjust;
		GetMesh()->SetRelativeLocation(MeshRelativeLocation);
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}

	K2_OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	CharacterEquipmentComponent->UnequipCurrentItem();
}

void AXyzBaseCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	RecalculateBaseEyeHeight();

	if (HalfHeightAdjust != 0.f && ScaledHalfHeightAdjust != 0.f)
	{
		const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
		FVector MeshRelativeLocation = GetMesh()->GetRelativeLocation();
		MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z;
		GetMesh()->SetRelativeLocation(MeshRelativeLocation);
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}

	K2_OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	EquipItemFromCurrentSlot();
}

void AXyzBaseCharacter::CrouchInternal() const
{
	BaseCharacterMovementComponent->Crouch();
}

void AXyzBaseCharacter::UnCrouchInternal() const
{
	BaseCharacterMovementComponent->UnCrouch();
	if (BaseCharacterMovementComponent->IsSwimming())
	{
		BaseCharacterMovementComponent->UpdateSwimmingCapsuleSize();
	}
}
#pragma endregion

#pragma region PRONE

bool AXyzBaseCharacter::IsProne() const
{
	return AbilitySystemComponent->IsAbilityActiveRemote(EGameplayAbility::Prone);
}

void AXyzBaseCharacter::Prone()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::Prone, EGameplayAbilityActivationFlag::TryActivateOnce);
}

void AXyzBaseCharacter::UnProne()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::Prone, EGameplayAbilityActivationFlag::TryCancelRepeat);
}

void AXyzBaseCharacter::ToggleProne()
{
	if (IsProne())
	{
		UnProne();
	}
	else
	{
		Prone();
	}
}

void AXyzBaseCharacter::OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	RecalculateBaseEyeHeight();

	if (HalfHeightAdjust != 0.f && ScaledHalfHeightAdjust != 0.f)
	{
		FVector MeshRelativeLocation = GetMesh()->GetRelativeLocation();
		MeshRelativeLocation.Z += HalfHeightAdjust;
		GetMesh()->SetRelativeLocation(MeshRelativeLocation);
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}

	CharacterEquipmentComponent->UnequipCurrentItem();
}

void AXyzBaseCharacter::OnStopProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	RecalculateBaseEyeHeight();

	if (HalfHeightAdjust != 0.f && ScaledHalfHeightAdjust != 0.f)
	{
		FVector MeshRelativeLocation = GetMesh()->GetRelativeLocation();
		MeshRelativeLocation.Z -= HalfHeightAdjust;
		GetMesh()->SetRelativeLocation(MeshRelativeLocation);
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
}

void AXyzBaseCharacter::ProneInternal() const
{
	BaseCharacterMovementComponent->Prone();
}

void AXyzBaseCharacter::UnProneInternal() const
{
	BaseCharacterMovementComponent->UnProne();
}
#pragma endregion

#pragma region MANTLING

void AXyzBaseCharacter::StartMantle()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::Mantle, EGameplayAbilityActivationFlag::TryActivateOnce);
}

void AXyzBaseCharacter::StopMantle()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::Mantle, EGameplayAbilityActivationFlag::TryCancelOnce);
}

bool AXyzBaseCharacter::DetectLedge(FLedgeDescription& LedgeDescription) const
{
	UWorld* World = GetWorld();
	float CurrentScaledCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector CharacterBottom = GetActorLocation() - CurrentScaledCapsuleHalfHeight * FVector::UpVector;
	ECollisionChannel CollisionChannel = ECC_Climbing;
	FCollisionQueryParams CollisionParams;
	CollisionParams.bTraceComplex = true;
	CollisionParams.AddIgnoredActor(this);
	float LedgeGeometryTolerance = 5.f;

	FHitResult ForwardHitResult;
	float ForwardCollisionCapsuleHalfHeight = (HighMantleSettings.MaxHeight - LowMantleSettings.MinHeight) / 2;
	FVector ForwardStartLocation = CharacterBottom + (LowMantleSettings.MinHeight + ForwardCollisionCapsuleHalfHeight - LedgeGeometryTolerance) * FVector::UpVector;
	if (BaseCharacterMovementComponent->IsSwimming())
	{
		ForwardCollisionCapsuleHalfHeight = (HighMantleSettings.MaxHeight - CachedCollisionCapsuleScaledHalfHeight) / 2;
		ForwardStartLocation = CharacterBottom + (CurrentScaledCapsuleHalfHeight + ForwardCollisionCapsuleHalfHeight - LedgeGeometryTolerance) * FVector::UpVector;
	}
	FVector ActorForwardVector = GetActorForwardVector();
	ActorForwardVector.Z = 0.f;
	FVector ForwardEndLocation = ForwardStartLocation + ForwardTraceDistance * ActorForwardVector.GetSafeNormal();
	FCollisionShape ForwardCollisionShape = FCollisionShape::MakeCapsule(CachedCollisionCapsuleScaledRadius, ForwardCollisionCapsuleHalfHeight);

	// Ensuring that nothing is blocking in the current position, e.g. ceiling
	if (World->OverlapBlockingTestByChannel(ForwardStartLocation, FQuat::Identity, CollisionChannel, ForwardCollisionShape, CollisionParams, FCollisionResponseParams::DefaultResponseParam))
	{
		return false;
	}

	if (World->SweepSingleByChannel(ForwardHitResult, ForwardStartLocation, ForwardEndLocation, FQuat::Identity, CollisionChannel, ForwardCollisionShape, CollisionParams, FCollisionResponseParams::DefaultResponseParam))
	{
		FHitResult DownwardHitResult;
		FCollisionShape DownwardCollisionShape = FCollisionShape::MakeSphere(CachedCollisionCapsuleScaledRadius);
		FVector DownwardStartLocation = ForwardHitResult.ImpactPoint - ForwardHitResult.ImpactNormal * MantlingDepthZ;
		DownwardStartLocation.Z = CharacterBottom.Z + HighMantleSettings.MaxHeight + LedgeGeometryTolerance;
		FVector DownwardEndLocation = DownwardStartLocation;
		DownwardEndLocation.Z = CharacterBottom.Z + LowMantleSettings.MinHeight - LedgeGeometryTolerance;
		if (BaseCharacterMovementComponent->IsSwimming())
		{
			DownwardEndLocation.Z = CharacterBottom.Z + CurrentScaledCapsuleHalfHeight - LedgeGeometryTolerance;
		}

		if (World->SweepSingleByChannel(DownwardHitResult, DownwardStartLocation, DownwardEndLocation, FQuat::Identity, CollisionChannel, DownwardCollisionShape, CollisionParams, FCollisionResponseParams::DefaultResponseParam))
		{
			FVector OverlapTestLocation = DownwardHitResult.ImpactPoint + (CachedCollisionCapsuleScaledHalfHeight + LedgeGeometryTolerance) * FVector::UpVector;

			if (!World->OverlapBlockingTestByProfile(OverlapTestLocation, FQuat::Identity, CollisionProfilePawn, GetCapsuleComponent()->GetCollisionShape(), CollisionParams))
			{
				LedgeDescription.TargetLocation = OverlapTestLocation - LedgeGeometryTolerance * FVector::UpVector;
				LedgeDescription.TargetRotation = FVector(-ForwardHitResult.ImpactNormal.X, -ForwardHitResult.ImpactNormal.Y, 0.f).ToOrientationRotator();
				LedgeDescription.LedgeNormal = ForwardHitResult.ImpactNormal.GetSafeNormal2D();
				LedgeDescription.LedgeActor = DownwardHitResult.Component.Get();

				return true;
			}
		}
	}

	return false;
}

void AXyzBaseCharacter::Mantle(bool bForceMantle /*= false*/)
{
	FLedgeDescription LedgeDescription;
	if (DetectLedge(LedgeDescription))
	{
		FMantlingMovementParameters MantlingParameters;
		MantlingParameters.InitialLocation = GetActorLocation() + (CachedCollisionCapsuleScaledHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * FVector::UpVector;
		MantlingParameters.InitialRotation = GetActorRotation();
		MantlingParameters.TargetLocation = LedgeDescription.TargetLocation;
		MantlingParameters.TargetRotation = LedgeDescription.TargetRotation;
		MantlingParameters.TargetActor = LedgeDescription.LedgeActor;

		float LedgeHeight = LedgeDescription.TargetLocation.Z - MantlingParameters.InitialLocation.Z;
		const FMantlingSettings& MantlingSettings = GetMantlingSettings(LedgeHeight);
		if (!MantlingSettings.MantlingMontage || !MantlingSettings.MantlingCurve)
		{
			return;
		}
		MantlingParameters.MantlingCurve = MantlingSettings.MantlingCurve;

		float MantlingHeight = (MantlingParameters.TargetLocation - MantlingParameters.InitialLocation).Z;
		float MinRange;
		float MaxRange;
		MantlingSettings.MantlingCurve->GetTimeRange(MinRange, MaxRange);
		MantlingParameters.Duration = MaxRange - MinRange;

		FVector2D SourceRange(MantlingSettings.MinHeight, MantlingSettings.MaxHeight);
		FVector2D TargetRange(MantlingSettings.MinHeightStartTime, MantlingSettings.MaxHeightStartTime);
		MantlingParameters.StartTime = FMath::GetMappedRangeValueClamped(SourceRange, TargetRange, MantlingHeight);
		MantlingParameters.InitialAnimationLocation = MantlingParameters.TargetLocation - MantlingSettings.AnimationCorrectionZ * FVector::UpVector + MantlingSettings.AnimationCorrectionXY * LedgeDescription.LedgeNormal;

		BaseCharacterMovementComponent->StartMantle(MantlingParameters);

		if (GetMesh()->GetAnimInstance())
		{
			GetMesh()->GetAnimInstance()->Montage_Play(MantlingSettings.MantlingMontage, 1.f, EMontagePlayReturnType::Duration, MantlingParameters.StartTime);
		}
		OnMantle(MantlingSettings, MantlingParameters);
	}
}

const FMantlingSettings& AXyzBaseCharacter::GetMantlingSettings(float LedgeHeight) const
{
	return LedgeHeight > LowMantleSettings.MaxHeight ? HighMantleSettings : LowMantleSettings;
}

void AXyzBaseCharacter::StartMantleInternal()
{
	Mantle();
}
#pragma endregion

#pragma region ENVIRONMENT ACTORS

void AXyzBaseCharacter::RegisterEnvironmentActor(AEnvironmentActor* EnvironmentActor)
{
	EnvironmentActors.AddUnique(EnvironmentActor);
}

void AXyzBaseCharacter::UnRegisterEnvironmentActor(AEnvironmentActor* EnvironmentActor)
{
	EnvironmentActors.RemoveSingleSwap(EnvironmentActor);
}

bool AXyzBaseCharacter::CanUseEnvironmentActors() const
{
	return EnvironmentActors.Num() > 0;
}

bool AXyzBaseCharacter::IsUsingEnvironmentActor() const
{
	return AbilitySystemComponent->IsAbilityActiveRemote(EGameplayAbility::UseEnvironmentActor);
}

void AXyzBaseCharacter::ToggleUseEnvironmentActor()
{
	if (IsUsingEnvironmentActor())
	{
		StopUseEnvironmentActor();
	}
	else
	{
		StartUseEnvironmentActor();
	}
}

void AXyzBaseCharacter::StartUseEnvironmentActor()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::UseEnvironmentActor, EGameplayAbilityActivationFlag::TryActivateOnce);
}

void AXyzBaseCharacter::StopUseEnvironmentActor()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::UseEnvironmentActor, EGameplayAbilityActivationFlag::TryCancelOnce);
}

bool AXyzBaseCharacter::TryAttachCharacterToLadder()
{
	ALadder* Ladder = GetAvailableLadder();
	if (IsValid(Ladder))
	{
		if (Ladder->IsOnTop())
		{
			PlayAnimMontage(Ladder->GetAttachFromTopAnimMontage());
			OnAttachedToLadderFromTop(Ladder);
		}

		BaseCharacterMovementComponent->AttachCharacterToLadder(Ladder);
		return true;
	}
	return false;
}

bool AXyzBaseCharacter::TryAttachCharacterToZipline()
{
	AZipline* Zipline = GetAvailableZipline();
	if (IsValid(Zipline))
	{
		BaseCharacterMovementComponent->AttachCharacterToZipline(Zipline);
		return true;
	}
	return false;
}

ALadder* AXyzBaseCharacter::GetAvailableLadder()
{
	for (AEnvironmentActor* EnvironmentActor : EnvironmentActors)
	{
		ALadder* Ladder = Cast<ALadder>(EnvironmentActor);
		if (IsValid(Ladder))
		{
			return Ladder;
		}
	}
	return nullptr;
}

AZipline* AXyzBaseCharacter::GetAvailableZipline()
{
	for (AEnvironmentActor* EnvironmentActor : EnvironmentActors)
	{
		AZipline* Zipline = Cast<AZipline>(EnvironmentActor);
		if (IsValid(Zipline))
		{
			return Zipline;
		}
	}
	return nullptr;
}

void AXyzBaseCharacter::StartUseEnvironmentActorInternal()
{
	if (!TryAttachCharacterToLadder() && !TryAttachCharacterToZipline())
	{
		StopUseEnvironmentActor();
	}
}

void AXyzBaseCharacter::StopUseEnvironmentActorInternal() const
{
	if (GetBaseCharacterMovementComponent()->IsOnLadder())
	{
		GetBaseCharacterMovementComponent()->DetachCharacterFromLadder(EDetachFromLadderMethod::JumpOff);
	}

	if (GetBaseCharacterMovementComponent()->IsOnZipline())
	{
		GetBaseCharacterMovementComponent()->DetachCharacterFromZipline(EDetachFromZiplineMethod::Fall);
	}
}
#pragma endregion

#pragma region WALL RUNNING

void AXyzBaseCharacter::JumpOffRunnableWall()
{
	Server_JumpOffRunnableWall();
	if (IsLocallyControlled())
	{
		Multicast_JumpOffRunnableWall();
	}
}

void AXyzBaseCharacter::Server_JumpOffRunnableWall_Implementation()
{
	if (!IsLocallyControlled())
	{
		Multicast_JumpOffRunnableWall();
	}
}

void AXyzBaseCharacter::Multicast_JumpOffRunnableWall_Implementation()
{
	if (BaseCharacterMovementComponent->IsWallRunning())
	{
		BaseCharacterMovementComponent->DetachCharacterFromRunnableWall(EDetachFromRunnableWallMethod::JumpOff);
	}
}

void AXyzBaseCharacter::OnCharacterCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Multicast_StartWallRun(Hit);
}

void AXyzBaseCharacter::Multicast_StartWallRun_Implementation(FHitResult HitResult)
{
	BaseCharacterMovementComponent->StartWallRun(HitResult);
}
#pragma endregion

#pragma region LANDING

void AXyzBaseCharacter::UpdateJumpApexHeight()
{
	CurrentJumpApexHeight = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void AXyzBaseCharacter::OnCharacterLanded(const FHitResult& Hit)
{
	float FallHeight = CurrentJumpApexHeight - Hit.ImpactPoint.Z;
	if (FallHeight >= HardLandMinHeight)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			if (FallDamageCurve)
			{
				float Damage = FallDamageCurve->GetFloatValue(FallHeight);
				AttributeSet->AddHealth(-Damage);
			}

			if (!IsDead())
			{
				StartHardLand();
			}
		}
	}
	else
	{
		// Delay equipping weapons when landing due to the EquipItemFromCurrentSlot ability being executed by the server only (characters can still be in the falling mode there)
		GetWorld()->GetTimerManager().SetTimer(LandEquipItemTimerHandle, [this] { EquipItemFromCurrentSlot(); }, LandEquipItemTimerLength, false);
	}
}

void AXyzBaseCharacter::StartHardLand()
{
	Multicast_StartHardLand();
}

void AXyzBaseCharacter::StopHardLand()
{
	bIsHardLanding = false;

	AController* CharacterController = GetController();
	if (IsValid(CharacterController))
	{
		CharacterController->SetIgnoreMoveInput(false);
	}
	EquipItemFromCurrentSlot();
}

void AXyzBaseCharacter::Multicast_StartHardLand_Implementation()
{
	if (!HardLandAnimMontage)
	{
		return;
	}

	bIsHardLanding = true;

	AController* CharacterController = GetController();
	if (IsValid(CharacterController))
	{
		CharacterController->SetIgnoreMoveInput(true);
	}

	GetMesh()->GetAnimInstance()->Montage_Play(HardLandAnimMontage);
	FOnMontageEnded OnHardLandMontageEnded;
	OnHardLandMontageEnded.BindLambda([this](UAnimMontage* Montage, bool bInterrupted) { StopHardLand(); });
	GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(OnHardLandMontageEnded, HardLandAnimMontage);
}
#pragma endregion

#pragma region ATTRIBUTES

bool AXyzBaseCharacter::IsDead() const
{
	return AbilitySystemComponent->IsAbilityActiveRemote(EGameplayAbility::Death);
}

bool AXyzBaseCharacter::IsOutOfStamina() const
{
	return AbilitySystemComponent->IsAbilityActiveRemote(EGameplayAbility::OutOfStamina);
}

void AXyzBaseCharacter::OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	AttributeSet->AddHealth(-Damage);
}

void AXyzBaseCharacter::OnDeath(bool bShouldPlayAnimMontage)
{
	if (bShouldPlayAnimMontage && DeathAnimMontage)
	{
		float Duration = PlayAnimMontage(DeathAnimMontage);
		if (Duration == 0.f)
		{
			EnableRagdoll();
		}
	}
	else
	{
		EnableRagdoll();
	}

	SetGameplayAbilityActivationFlag(EGameplayAbility::Death, EGameplayAbilityActivationFlag::TryActivateRepeat);
}

void AXyzBaseCharacter::OnOutOfStamina(bool bIsOutOfStamina) const
{
	AbilitySystemComponent->ActivateAbility(EGameplayAbility::OutOfStamina, bIsOutOfStamina);
}

void AXyzBaseCharacter::StartOutOfStaminaInternal()
{
	CharacterEquipmentComponent->UnequipCurrentItem();
	BaseCharacterMovementComponent->SetMovementFlag((uint32)EGameplayAbility::OutOfStamina, true);
}

void AXyzBaseCharacter::StopOutOfStaminaInternal()
{
	EquipItemFromCurrentSlot();
	BaseCharacterMovementComponent->SetMovementFlag((uint32)EGameplayAbility::OutOfStamina, false);
}

void AXyzBaseCharacter::OnOutOfOxygen(bool bIsOutOfOxygen) const
{
	AbilitySystemComponent->ApplyEffectToSelf(EGameplayEffect::OutOfOxygen, bIsOutOfOxygen);
}
#pragma endregion

#pragma region EQUIPMENT

void AXyzBaseCharacter::DrawNextItem()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::DrawNextItem, EGameplayAbilityActivationFlag::TryActivateOnce);
}

void AXyzBaseCharacter::DrawPreviousItem()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::DrawPreviousItem, EGameplayAbilityActivationFlag::TryActivateOnce);
}

void AXyzBaseCharacter::EquipItemFromCurrentSlot(bool bShouldSkipAnimation/* = false*/)
{
	EGameplayAbility Ability = bShouldSkipAnimation ? EGameplayAbility::EquipFromCurrentSlotFast : EGameplayAbility::EquipFromCurrentSlot;
	SetGameplayAbilityActivationFlag(Ability, EGameplayAbilityActivationFlag::TryActivateOnce);
}

bool AXyzBaseCharacter::IsPrimaryItemDrawn() const
{
	return AbilitySystemComponent->IsAbilityActiveRemote(EGameplayAbility::DrawPrimaryItem);
}

float AXyzBaseCharacter::GetCurrentThrowItemMovementSpeed() const
{
	return CharacterEquipmentComponent->GetCurrentThrowableWalkSpeed();
}

void AXyzBaseCharacter::TogglePrimaryItem()
{
	EGameplayAbilityActivationFlag Flag = IsPrimaryItemDrawn() ? EGameplayAbilityActivationFlag::TryCancelOnce : EGameplayAbilityActivationFlag::TryActivateOnce;
	SetGameplayAbilityActivationFlag(EGameplayAbility::DrawPrimaryItem, Flag);
}

void AXyzBaseCharacter::SheathePrimaryItem()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::DrawPrimaryItem, EGameplayAbilityActivationFlag::TryCancelOnce);
}

bool AXyzBaseCharacter::IsThrowingItem() const
{
	return AbilitySystemComponent->IsAbilityActiveRemote(EGameplayAbility::ThrowItem);
}

void AXyzBaseCharacter::StartItemThrow()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::ThrowItem, EGameplayAbilityActivationFlag::TryActivateOnce);
}

void AXyzBaseCharacter::StopItemThrow()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::ThrowItem, EGameplayAbilityActivationFlag::TryCancelOnce);
}

void AXyzBaseCharacter::DrawPrimaryItemInternal() const
{
	CharacterEquipmentComponent->EquipPrimaryItem();
}

void AXyzBaseCharacter::SheathePrimaryItemInternal() const
{
	CharacterEquipmentComponent->UnequipPrimaryItem();
}

void AXyzBaseCharacter::StartItemThrowInternal()
{
	AThrowableItem* CurrentThrowable = CharacterEquipmentComponent->GetCurrentThrowableItem();
	if (IsValid(CurrentThrowable))
	{
		if (IsLocallyControlled() && !OnThrowItemDelegateHandle.IsValid())
		{
			OnThrowItemDelegateHandle = CurrentThrowable->OnThrowAnimationFinishedEvent.AddLambda([this]
			{
				StopItemThrow();
				SheathePrimaryItem();
			});
		}

		BaseCharacterMovementComponent->SetMovementFlag((uint32)EGameplayAbility::ThrowItem, true);
		CharacterEquipmentComponent->ThrowItem();
	}
}

void AXyzBaseCharacter::StopItemThrowInternal()
{
	AThrowableItem* CurrentThrowable = CharacterEquipmentComponent->GetCurrentThrowableItem();
	if (IsValid(CurrentThrowable))
	{
		if (CurrentThrowable->OnThrowAnimationFinishedEvent.Remove(OnThrowItemDelegateHandle))
		{
			OnThrowItemDelegateHandle.Reset();
		}
	}
	BaseCharacterMovementComponent->SetMovementFlag((uint32)EGameplayAbility::ThrowItem, false);
}
#pragma endregion

#pragma region AIMING

bool AXyzBaseCharacter::IsAiming() const
{
	return AbilitySystemComponent->IsAbilityActiveRemote(EGameplayAbility::AimWeapon);
}

void AXyzBaseCharacter::StartAiming()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::AimWeapon, EGameplayAbilityActivationFlag::ActivateRepeat);
}

void AXyzBaseCharacter::StopAiming()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::AimWeapon, EGameplayAbilityActivationFlag::TryCancelOnce);
}

void AXyzBaseCharacter::StartAimingInternal()
{
	// Handling cases when the character is already trying to fire a weapon
	// A delay is needed to let the animation transition to the aiming state
	DelayWeaponFire(DelayWeaponFireOnAiming);

	const AEquipmentItem* EquipmentItem = CharacterEquipmentComponent->GetCurrentEquipmentItem();
	if (IsValid(EquipmentItem))
	{
		BaseCharacterMovementComponent->SetMovementFlag((uint32)EGameplayAbility::AimWeapon, true);
		BaseCharacterMovementComponent->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
		CurrentAimingMovementSpeed = EquipmentItem->GetAimingWalkSpeed();
	}

	if (OnAimingEvent.IsBound())
	{
		OnAimingEvent.Broadcast(true);
	}

	// Cancel fire and try to restart it
	if (IsFiringWeapon())
	{
		StopWeaponFire();
		StartWeaponFire();
	}

	OnStartAiming();
}

void AXyzBaseCharacter::StopAimingInternal()
{
	BaseCharacterMovementComponent->SetMovementFlag((uint32)EGameplayAbility::AimWeapon, false);
	BaseCharacterMovementComponent->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	CurrentAimingMovementSpeed = BaseCharacterMovementComponent->MaxWalkSpeed;

	if (OnAimingEvent.IsBound())
	{
		OnAimingEvent.Broadcast(false);
	}

	OnStopAiming();
}
#pragma endregion

#pragma region RANGED WEAPONS

bool AXyzBaseCharacter::IsFiringWeapon() const
{
	return AbilitySystemComponent->IsAbilityActiveRemote(EGameplayAbility::FireWeapon);
}

void AXyzBaseCharacter::StartWeaponFire()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::FireWeapon, EGameplayAbilityActivationFlag::ActivateRepeat);
}

void AXyzBaseCharacter::StopWeaponFire()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::FireWeapon, EGameplayAbilityActivationFlag::TryCancelOnce);
}

void AXyzBaseCharacter::ActivateNextWeaponMode() const
{
	CharacterEquipmentComponent->ActivateNextWeaponMode();
}

bool AXyzBaseCharacter::IsReloadingWeapon() const
{
	return AbilitySystemComponent->IsAbilityActiveRemote(EGameplayAbility::ReloadWeapon);
}

float AXyzBaseCharacter::GetCurrentReloadingWalkSpeed() const
{
	return CharacterEquipmentComponent->GetCurrentWeaponReloadWalkSpeed();
}

void AXyzBaseCharacter::StartWeaponReload()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::ReloadWeapon, EGameplayAbilityActivationFlag::TryActivateOnce);
}

void AXyzBaseCharacter::StartWeaponAutoReload()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::ReloadWeapon, EGameplayAbilityActivationFlag::ActivateRepeat);
}

void AXyzBaseCharacter::StopWeaponReload()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::ReloadWeapon, EGameplayAbilityActivationFlag::TryCancelOnce);
}

void AXyzBaseCharacter::StartWeaponFireInternal()
{
	ARangedWeaponItem* CurrentRangedWeapon = CharacterEquipmentComponent->GetCurrentRangedWeapon();
	if (IsValid(CurrentRangedWeapon))
	{
		const FWeaponModeParameters* ModeParameters = CurrentRangedWeapon->GetWeaponModeParameters();
		if (ModeParameters && ModeParameters->FireMode == EWeaponFireMode::Single)
		{
			CurrentRangedWeapon->StartFire();
			FDelegateHandle OnShotEndDelegateHandle = CurrentRangedWeapon->OnShotEndEvent.AddLambda([this, CurrentRangedWeapon, OnShotEndDelegateHandle]
			{
				StopWeaponFire();
				if (IsValid(CurrentRangedWeapon))
				{
					CurrentRangedWeapon->OnShotEndEvent.Remove(OnShotEndDelegateHandle);
				}
			});
		}
	}
}

void AXyzBaseCharacter::StopWeaponFireInternal() const
{
	ARangedWeaponItem* CurrentRangedWeapon = CharacterEquipmentComponent->GetCurrentRangedWeapon();
	if (IsValid(CurrentRangedWeapon))
	{
		CurrentRangedWeapon->StopFire();
	}
}

void AXyzBaseCharacter::StartWeaponReloadInternal()
{
	if (CharacterEquipmentComponent->TryReloadCurrentWeapon())
	{
		BaseCharacterMovementComponent->SetMovementFlag((uint32)EGameplayAbility::ReloadWeapon, true);
	}
	else
	{
		StopWeaponReload();
	}
}

void AXyzBaseCharacter::StopWeaponReloadInternal() const
{
	ARangedWeaponItem* CurrentRangedWeapon = CharacterEquipmentComponent->GetCurrentRangedWeapon();
	if (IsValid(CurrentRangedWeapon))
	{
		CurrentRangedWeapon->StopReload();
		BaseCharacterMovementComponent->SetMovementFlag((uint32)EGameplayAbility::ReloadWeapon, false);
	}
}

void AXyzBaseCharacter::DelayWeaponFire(float DelayLength)
{
	if (GetWorld()->GetTimerManager().GetTimerRemaining(DelayWeaponFireTimerHandle) > DelayLength)
	{
		return;
	}

	EGameplayAbilityActivationFlag Flag = AbilityActivationFlags[(uint32)EGameplayAbility::FireWeapon];
	if (Flag == EGameplayAbilityActivationFlag::TryActivateRepeat || Flag == EGameplayAbilityActivationFlag::ActivateRepeat)
	{
		StopWeaponFire();
		GetWorld()->GetTimerManager().SetTimer(DelayWeaponFireTimerHandle, [this] { StartWeaponFire(); }, DelayLength, false);
	}
}
#pragma endregion

#pragma region MELEE WEAPONS

void AXyzBaseCharacter::StartPrimaryMeleeAttack()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::PrimaryMeleeAttack, EGameplayAbilityActivationFlag::TryActivateOnce);
}

void AXyzBaseCharacter::StopPrimaryMeleeAttack()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::PrimaryMeleeAttack, EGameplayAbilityActivationFlag::TryCancelOnce);
}

void AXyzBaseCharacter::StartSecondaryMeleeAttack()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::SecondaryMeleeAttack, EGameplayAbilityActivationFlag::TryActivateOnce);
}

void AXyzBaseCharacter::StopSecondaryMeleeAttack()
{
	SetGameplayAbilityActivationFlag(EGameplayAbility::SecondaryMeleeAttack, EGameplayAbilityActivationFlag::TryCancelOnce);
}

void AXyzBaseCharacter::StartPrimaryMeleeAttackInternal()
{
	AMeleeWeaponItem* CurrentMeleeWeapon = CharacterEquipmentComponent->GetCurrentMeleeWeapon();
	if (IsValid(CurrentMeleeWeapon))
	{
		if (IsLocallyControlled())
		{
			if (OnMeleeAttackActivatedDelegateHandle.IsValid())
			{
				CurrentMeleeWeapon->OnAttackActivatedEvent.Remove(OnMeleeAttackActivatedDelegateHandle);
			}

			OnMeleeAttackActivatedDelegateHandle = CurrentMeleeWeapon->OnAttackActivatedEvent.AddLambda([this](bool bIsActivated)
			{
				if (!bIsActivated)
				{
					StopPrimaryMeleeAttack();
				}
			});
		}
		CurrentMeleeWeapon->StartAttack(EMeleeAttackType::PrimaryAttack);
	}
}

void AXyzBaseCharacter::StopPrimaryMeleeAttackInternal()
{
	AMeleeWeaponItem* CurrentMeleeWeapon = CharacterEquipmentComponent->GetCurrentMeleeWeapon();
	if (IsValid(CurrentMeleeWeapon))
	{
		if (CurrentMeleeWeapon->OnAttackActivatedEvent.Remove(OnMeleeAttackActivatedDelegateHandle))
		{
			OnThrowItemDelegateHandle.Reset();
		}
	}
}

void AXyzBaseCharacter::StartSecondaryMeleeAttackInternal()
{
	AMeleeWeaponItem* CurrentMeleeWeapon = CharacterEquipmentComponent->GetCurrentMeleeWeapon();
	if (IsValid(CurrentMeleeWeapon))
	{
		if (IsLocallyControlled())
		{
			if (OnMeleeAttackActivatedDelegateHandle.IsValid())
			{
				CurrentMeleeWeapon->OnAttackActivatedEvent.Remove(OnMeleeAttackActivatedDelegateHandle);
			}

			OnMeleeAttackActivatedDelegateHandle = CurrentMeleeWeapon->OnAttackActivatedEvent.AddLambda([this](bool bIsActivated)
			{
				if (!bIsActivated)
				{
					StopSecondaryMeleeAttack();
				}
			});
		}
		CurrentMeleeWeapon->StartAttack(EMeleeAttackType::SecondaryAttack);
	}
}

void AXyzBaseCharacter::StopSecondaryMeleeAttackInternal()
{
	AMeleeWeaponItem* CurrentMeleeWeapon = CharacterEquipmentComponent->GetCurrentMeleeWeapon();
	if (IsValid(CurrentMeleeWeapon))
	{
		if (CurrentMeleeWeapon->OnAttackActivatedEvent.Remove(OnMeleeAttackActivatedDelegateHandle))
		{
			OnThrowItemDelegateHandle.Reset();
		}
	}
}
#pragma endregion
