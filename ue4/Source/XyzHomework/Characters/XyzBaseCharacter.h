// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "SignificanceManager.h"
#include "XyzGenericStructs.h"
#include "GameFramework/Character.h"
#include "Subsystems/SaveSubsystem/SaveSubsystemInterface.h"
#include "XyzBaseCharacter.generated.h"

class AEquipmentItem;
class AEnvironmentActor;
class ALadder;
class AXyzPlayerController;
class AZipline;
class IInteractable;
class UCharacterEquipmentComponent;
class UCharacterInventoryComponent;
class UDataTable;
class UGameplayAbility;
class UGameplayEffect;
class UInventoryItem;
class UInventorySlot;
class UWidgetComponent;
class UXyzAbilitySystemComponent;
class UXyzBaseCharMovementComponent;
class UXyzCharacterAttributeSet;

typedef TArray<AEnvironmentActor*, TInlineAllocator<10>> TInteractiveActorsArray;

/** Flags used by AXyzBaseCharacter to control the activation/deactivation behavior of character gameplay abilities. */
UENUM()
enum class EGameplayAbilityActivationFlag : uint8
{
	None = 0,
	TryActivateOnce,
	TryActivateRepeat,
	ActivateRepeat,
	TryCancelOnce,
	TryCancelRepeat,
	CancelRepeat,
	Max UMETA(Hidden)
};

/** Extended ACharacter class used as a base of all humanoid characters. Consists mainly of the custom movement, equipment, and inventory components. Capable of managing gameplay abilities and effects via GAS. */
UCLASS(Abstract, NotBlueprintable)
class XYZHOMEWORK_API AXyzBaseCharacter : public ACharacter, public IGenericTeamAgentInterface, public ISaveSubsystemInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

	friend class UGA_CharacterAbilityBase;

public:
	explicit AXyzBaseCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	bool IsFirstPerson() const { return bIsFirstPerson; }
	FRotator GetAimOffset() const;
	bool IsAnimMontagePlaying() const;
	void EnableRagdoll() const;

	//@ IGenericTeamAgentInterface	
	virtual FGenericTeamId GetGenericTeamId() const override;
	//~ IGenericTeamAgentInterface

	//@ SaveSubsystemInterface
	virtual void OnLevelDeserialized_Implementation() override;
	//~ SaveSubsystemInterface

protected:
	virtual void RecalculateBaseEyeHeight() override;
	void SetupProgressBarWidget();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character", meta = (ClampMin = 1.f, UIMin = 1.f))
	float CollisionCapsuleHalfHeight = 90.f;
	/** Team this character belongs to. Affects AI hostility towards this character. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character")
	ETeam Team = ETeam::Enemy;

	TWeakObjectPtr<AXyzPlayerController> CachedPlayerController;
	/** Enables first-person animations and game logic (compatible only with FPPlayerCharacter).*/
	bool bIsFirstPerson = false;
	float CachedCollisionCapsuleScaledRadius = 0.f;
	float CachedCollisionCapsuleScaledHalfHeight = 0.f;

#pragma region SIGNIFICANCE MANAGER

protected:
	void EnableSignificance();
	float SignificanceFunction(USignificanceManager::FManagedObjectInfo* ObjectInfo, FTransform ViewPoint);
	void PostSignificanceFunction(USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSignificance, float Significance, bool bFinal);
	void SetSignificanceSettings(AXyzBaseCharacter* Character, float MovementComponentTickInterval, bool bWidgetComponentVisibility, float AIControllerTickInterval, bool bMeshTickEnabled, float MeshTickInterval = 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Significance Manager", meta = (ClampMin = 0.f, UIMin = 0.f))
	bool bIsSignificanceEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Significance Manager", meta = (ClampMin = 0.f, UIMin = 0.f))
	float VeryHighSignificanceDistance = 10000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Significance Manager", meta = (ClampMin = 0.f, UIMin = 0.f))
	float HighSignificanceDistance = 20000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Significance Manager", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MediumSignificanceDistance = 30000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Significance Manager", meta = (ClampMin = 0.f, UIMin = 0.f))
	float LowSignificanceDistance = 40000.f;
#pragma endregion

#pragma region GAMEPLAY ABILITY SYSTEM

public:
	UXyzCharacterAttributeSet* GetCharacterAttributes() const { return AttributeSet; }
	/** Sets a new ability activation flag that will be processed in TryActivateGameplayAbilitiesWithFlags() during the next tick. */
	void SetGameplayAbilityActivationFlag(EGameplayAbility Ability, EGameplayAbilityActivationFlag ActivationFlag);

	// IAbilitySystemInterface	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ IAbilitySystemInterface

protected:
	/** Binds ability activation/deactivation functions to an array of callbacks accessed by UGA_CharacterAbilityBase via ExecuteGameplayAbilityCallbackInternal(). */
	void InitializeGameplayAbilityCallbacks();
	/** Iterates through predefined AbilityActivationFlags and triggers ability activation/deactivation in GAS. */
	void TryActivateGameplayAbilitiesWithFlags();
	/**
	 * Executes the actual activation/deactivation logic of abilities.
	 * @param bIsServerCall Was this ability activation initiated by the server?
	 */
	void ExecuteGameplayAbilityCallbackInternal(EGameplayAbility Ability, bool bIsActivationCallback, bool bIsServerCall = true);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExecuteGameplayAbilityCallback(EGameplayAbility Ability, bool bIsActivationCallback);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UXyzAbilitySystemComponent* AbilitySystemComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UXyzCharacterAttributeSet* AttributeSet;

	/** Flags controlling the activation/deactivation behavior of character gameplay abilities. */
	TArray<EGameplayAbilityActivationFlag> AbilityActivationFlags;
	TArray<TFunction<void()>> AbilityActivationCallbacks;
	TArray<TFunction<void()>> AbilityDeactivationCallbacks;
#pragma endregion

#pragma region INVENTORY

public:
	UCharacterInventoryComponent* GetCharacterInventoryComponent() const { return CharacterInventoryComponent; }
	void TogglePlayerMouseInput(APlayerController* PlayerController);
	void UpdatePlayerMouseInput(APlayerController* PlayerController) const;
	/** Toggles visibility of the inventory menu. */
	void UseInventory(APlayerController* PlayerController) const;
	/** Toggles visibility of the radial menu. */
	void UseRadialMenu(APlayerController* PlayerController) const;
	/** Tries to add a new item of type ItemType to the inventory. */
	bool PickupItem(EInventoryItemType ItemType, int32 Amount) const;
	/** Tries to remove an item of type ItemType from the inventory. If found, its pickup version will be spawned under the character's feet. */
	void DropItem(EInventoryItemType ItemType, int32 Amount) const;
	/** If consumable, the item will be consumed. If equipment item, tries to remove the item from inventory and add it to the equipment list. */
	UFUNCTION(Server, Reliable)
	void Server_UseItem(UInventoryItem* InventoryItem, UInventorySlot* InventorySlot = nullptr);
	/**
	 * Tries to add this item to the equipment. If failed, tries to add it to the inventory. If failed again, drops the item.
	 * @param EquipmentSlotIndex Equipment slot in which the item will be placed. -1 indicates the request to find a compatible slot.
	 */
	UFUNCTION(Server, Reliable)
	void Server_AddEquipmentItem(EInventoryItemType ItemType, int32 Amount = 1, int32 EquipmentSlotIndex = -1);
	/** Removes an item of type Equipment from the equipment. Then tries to add it to the inventory, and, if failed, drops the item. */
	UFUNCTION(Server, Reliable)
	void Server_RemoveEquipmentItem(int32 EquipmentSlotIndex);
	UFUNCTION(Server, Reliable)
	void Server_DropItem(EInventoryItemType ItemType, int32 Amount);
	/** Adds an item of type Ammo to inventory. */
	bool AddAmmoToInventory(EWeaponAmmoType AmmoType, int32 Amount) const;
	/** Removes an item of type Ammo from inventory. */
	int32 RemoveAmmoFromInventory(EWeaponAmmoType AmmoType, int32 Amount) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCharacterInventoryComponent* CharacterInventoryComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character")
	TSoftObjectPtr<UDataTable> InventoryItemDataTable;
#pragma endregion

#pragma region INTERACTIVE OBJECTS

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractableFound, FName)
	FOnInteractableFound OnInteractiveObjectFound;

	/** Interacts with a smart object. */
	void InteractWithObject();

protected:
	UFUNCTION(Server, Reliable)
	void Server_InteractWithObject(UObject* InteractableObject);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_InteractWithObject(UObject* InteractableObject);
	/** Detects interactive objects in front of the character. */
	void LineTraceInteractiveObject();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character", meta = (ClampMin = 1.f, UIMin = 1.f))
	float InteractiveObjectDetectionRange = 500.f;

	TScriptInterface<IInteractable> CurrentInteractableObject;
#pragma endregion

#pragma region INVERSE KINEMATICS

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Character|Inverse Kinematics")
	float GetIKLeftFootOffsetZ() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Character|Inverse Kinematics")
	float GetIKRightFootOffsetZ() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Character|Inverse Kinematics")
	float GetIKPelvisOffsetZ() const;

protected:
	void UpdateIKSettings(float DeltaSeconds);
	float GetIKOffsetForFootSocketZ(FName SocketName) const;
	float GetIKOffsetForPelvisSocketZ() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Inverse Kinematics")
	FName LeftFootSocketName = "foot_l_socket";
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Inverse Kinematics")
	FName RightFootSocketName = "foot_r_socket";
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Inverse Kinematics", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxFeetIKOffsetZ = 50.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Inverse Kinematics", meta = (ClampMin = 0.f, UIMin = 0.f))
	float IKInterpSpeed = 15.f;

	float IKLeftFootOffset = 0.f;
	float IKRightFootOffset = 0.f;
	float IKPelvisOffset = 0.f;
#pragma endregion

#pragma region MOVEMENT

public:
	UXyzBaseCharMovementComponent* GetBaseCharacterMovementComponent() const { return BaseCharacterMovementComponent; }
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	/** Activates the ability via GAS. */
	virtual void StartJump();
	virtual void MoveForward(float Value) {}
	virtual void MoveRight(float Value) {}
	virtual void Turn(float Value) {}
	virtual void LookUp(float Value) {}
	virtual void TurnAtRate(float Value) {}
	virtual void LookUpAtRate(float Value) {}

protected:
	/** Actual activation logic of the gameplay ability. */
	void StartJumpInternal();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character|Movement", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BaseTurnRate = 45.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character|Movement", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BaseLookUpRate = 45.f;

	/** Movement component that replaces the default character movement component. */
	UPROPERTY()
	UXyzBaseCharMovementComponent* BaseCharacterMovementComponent;
#pragma endregion

#pragma region SWIMMING

public:
	virtual void SwimForward(float Value) {}
	virtual void SwimRight(float Value) {}
	virtual void SwimUp(float Value) {}
	/** Is the gameplay ability currently active? (activation flag replicated from server) */
	bool IsDiveAbilityActive() const;
	/** Activates the ability via GAS. */
	void StartDive();
	/** Deactivates the ability via GAS. */
	void StopDive();
	/** Is the gameplay effect currently applied? (flag replicated from server) */
	bool IsDiving() const;
	/** Applies/removes the effect via GAS. */
	void OnDiving(bool bIsDiving) const;

protected:
	/** Actual activation logic of the gameplay ability. */
	void StartDiveInternal();
	/** Actual deactivation logic of the gameplay ability. */
	void StopDiveInternal() const;

	UPROPERTY(EditAnywhere, Category = "Base Character|Movement", meta = (ClampMin = 0.f, UIMin = 0.f))
	float DiveAbilityLength = .5f;

	FTimerHandle DiveTimerHandle;
#pragma endregion

#pragma region SPRINTING

public:
	/** Is the gameplay ability currently active? (activation flag replicated from server) */
	bool IsSprinting() const;
	/** Activates the ability via GAS. */
	virtual void StartSprint();
	/** Deactivates the ability via GAS. */
	void StopSprint();
	UFUNCTION(BlueprintNativeEvent, Category = "Base Character|Movement")
	void OnStartSprint();
	UFUNCTION(BlueprintNativeEvent, Category = "Base Character|Movement")
	void OnStopSprint();

protected:
	/** Actual activation logic of the gameplay ability. */
	void StartSprintInternal();
	/** Actual deactivation logic of the gameplay ability. */
	void StopSprintInternal() const;
	void OnStartSprint_Implementation();
	void OnStopSprint_Implementation();
#pragma endregion

#pragma region SLIDING

public:
	/** Is the gameplay ability currently active? (activation flag replicated from server) */
	bool IsSliding() const;
	/** Activates the ability via GAS. */
	void StartSlide();
	/** Deactivates the ability via GAS. */
	void StopSlide();
	virtual void OnStartSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	virtual void OnStopSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

protected:
	/** Actual activation logic of the gameplay ability. */
	void StartSlideInternal() const;
	/** Actual deactivation logic of the gameplay ability. */
	void StopSlideInternal();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character|Movement")
	UAnimMontage* SlideAnimMontage;
#pragma endregion

#pragma region CROUCHING

public:
	/** Is the gameplay ability currently active? (activation flag replicated from server) */
	bool IsCrouching() const;
	virtual bool CanChangeCrouchState() const { return true; }
	/** Activates the ability via GAS. */
	virtual void Crouch(bool bClientSimulation = false) override;
	/** Deactivates the ability via GAS. */
	virtual void UnCrouch(bool bClientSimulation = false) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

protected:
	/** Actual activation logic of the gameplay ability. */
	void CrouchInternal() const;
	/** Actual deactivation logic of the gameplay ability. */
	void UnCrouchInternal() const;
#pragma endregion

#pragma region PRONE

public:
	/** Is the gameplay ability currently active? (activation flag replicated from server) */
	virtual bool IsProne() const;
	virtual bool CanChangeProneState() const { return true; }
	/** Activates the ability via GAS. */
	void Prone();
	/** Deactivates the ability via GAS. */
	void UnProne();
	/** Activates/deactivates the ability via GAS. */
	void ToggleProne();
	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	virtual void OnStopProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

protected:
	/** Actual activation logic of the gameplay ability. */
	void ProneInternal() const;
	/** Actual deactivation logic of the gameplay ability. */
	void UnProneInternal() const;

	float ProneEyeHeight = 0.f;
#pragma endregion

#pragma region MANTLING

public:
	/** Activates the ability via GAS. */
	void StartMantle();
	/** Deactivates the ability via GAS. */
	void StopMantle();
	/**
	 * Detects a mountable ledge according to HighMantleSettings and LowMantleSettings.
	 * @param LedgeDescription Ledge description calculated during the detection.
	 */
	bool DetectLedge(OUT FLedgeDescription& LedgeDescription) const;
	/** Tries to detect a mountable ledge. If succeeds, starts mantling. */
	UFUNCTION(BlueprintCallable, Category = "Base Character|Movement")
	void Mantle(bool bForceMantle = false);

protected:
	const FMantlingSettings& GetMantlingSettings(float LedgeHeight) const;
	/** Actual activation logic of the gameplay ability. */
	void StartMantleInternal();
	virtual void OnMantle(const FMantlingSettings& MantlingSettings, const FMantlingMovementParameters& MantlingParameters) {}
	void DrawLedgeDetectionDebugCapsules(FHitResult HitResult, float TestCapsuleHalfHeight, float TestCapsuleRadius, FVector TestStartLocation, FVector TestEndLocation, bool bTestResult, float DrawTime, FColor TestColor = FColor::Black, FColor HitColor = FColor::Yellow) const;

	/** Tolerance of the sweep overlap test. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Movement|Mantling", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MantlingDepthZ = 10.f;
	/** Mountable ledge detection range. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Movement|Mantling", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ForwardTraceDistance = 100.f;
	/** Highest ledge that the character can mantle. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Movement|Mantling")
	FMantlingSettings HighMantleSettings;
	/** Lowest ledge that the character can mantle. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Movement|Mantling")
	FMantlingSettings LowMantleSettings;

	mutable float CachedDistanceToFloorZ = 2.f;
#pragma endregion

#pragma region ENVIRONMENT ACTORS

public:
	/** Adds this environment actor to the list of usable actors. */
	void RegisterEnvironmentActor(AEnvironmentActor* EnvironmentActor);
	/** Removes this environment actor from the list of usable actors. */
	void UnRegisterEnvironmentActor(AEnvironmentActor* EnvironmentActor);
	/** Is the gameplay ability currently active? (activation flag replicated from server) */
	bool IsUsingEnvironmentActor() const;
	bool CanUseEnvironmentActors() const;
	/** Activates/deactivates the ability via GAS. */
	void ToggleUseEnvironmentActor();
	/** Activates the ability via GAS. */
	void StartUseEnvironmentActor();
	/** Deactivates the ability via GAS. */
	void StopUseEnvironmentActor();
	bool TryAttachCharacterToZipline();
	bool TryAttachCharacterToLadder();
	virtual void ClimbLadderUp(float Value) {}

protected:
	ALadder* GetAvailableLadder();
	AZipline* GetAvailableZipline();
	/** Actual activation logic of the gameplay ability. */
	void StartUseEnvironmentActorInternal();
	/** Actual deactivation logic of the gameplay ability. */
	void StopUseEnvironmentActorInternal() const;
	/** Logic executed when the character stars going down a ladder. */
	virtual void OnAttachedToLadderFromTop(ALadder* Ladder) {}

	/** List of currently available environment actors.*/
	TInteractiveActorsArray EnvironmentActors;
#pragma endregion

#pragma region WALL RUNNING

public:
	virtual void WallRun(float Value) {}
	virtual void OnWallRunStart();
	virtual void OnWallRunEnd() {}
	void JumpOffRunnableWall();
	UFUNCTION(Server, Reliable)
	void Server_JumpOffRunnableWall();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_JumpOffRunnableWall();

protected:
	UFUNCTION()
	void OnCharacterCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartWallRun(FHitResult HitResult);

	FTimerHandle WallRunEquipItemTimerHandle;
	/** Delay before a currently selected equipment item can be equipped back. */
	float WallRunEquipItemTimerLength = 1.f;
#pragma endregion

#pragma region LANDING

protected:
	UFUNCTION()
	void UpdateJumpApexHeight();
	UFUNCTION()
	void OnCharacterLanded(const FHitResult& Hit);
	virtual void StartHardLand();
	virtual void StopHardLand();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartHardLand();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Movement|Landing", meta = (ClampMin = 0.f, UIMin = 0.f))
	float HardLandMinHeight = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Movement|Landing")
	UCurveFloat* FallDamageCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Movement|Landing")
	UAnimMontage* HardLandAnimMontage;

	bool bIsHardLanding = false;
	float CurrentJumpApexHeight = 0.f;
	FTimerHandle LandEquipItemTimerHandle;
	/** Delay before a currently selected equipment item can be equipped back. */
	float LandEquipItemTimerLength = .2f;
#pragma endregion

#pragma region ATTRIBUTES

public:
	UWidgetComponent* GetCharacterWidgetComponent() const { return WidgetComponent; }
	/** Is the gameplay ability currently active? (activation flag replicated from server) */
	UFUNCTION(BlueprintCallable, Category = "Base Character")
	bool IsDead() const;
	/** Is the gameplay ability currently active? (activation flag replicated from server) */
	bool IsOutOfStamina() const;

protected:
	/** Updates the Health value in the attribute set. */
	UFUNCTION()
	void OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	/** Activates the ability via GAS. */
	virtual void OnDeath(bool bShouldPlayAnimMontage);
	/**
	 * Activates/deactivates the ability via GAS.
	 * @param bIsOutOfStamina Activation flag.
	 */
	void OnOutOfStamina(bool bIsOutOfStamina) const;
	/** Actual activation logic of the gameplay ability. */
	virtual void StartOutOfStaminaInternal();
	/** Actual deactivation logic of the gameplay ability. */
	virtual void StopOutOfStaminaInternal();
	/**
	 * Applies/removes the effect via GAS.
	 * @param bIsOutOfOxygen Activation flag.
	 */
	void OnOutOfOxygen(bool bIsOutOfOxygen) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* WidgetComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character")
	UAnimMontage* DeathAnimMontage;
#pragma endregion

#pragma region EQUIPMENT

public:
	UCharacterEquipmentComponent* GetCharacterEquipmentComponent() const { return CharacterEquipmentComponent; }
	/** Activates the ability via GAS. */
	void DrawNextItem();
	/** Activates the ability via GAS. */
	void DrawPreviousItem();
	/** Activates the ability via GAS. */
	void EquipItemFromCurrentSlot(bool bShouldSkipAnimation = false);
	/** Is the gameplay ability currently active? (activation flag replicated from server) */
	bool IsPrimaryItemDrawn() const;
	float GetCurrentThrowItemMovementSpeed() const;
	/** Activates the ability via GAS. */
	void TogglePrimaryItem();
	/** Deactivates the ability via GAS. */
	void SheathePrimaryItem();
	/** Is the gameplay ability currently active? (activation flag replicated from server) */
	bool IsThrowingItem() const;
	/** Activates the ability via GAS. */
	void StartItemThrow();
	/** Deactivates the ability via GAS. */
	void StopItemThrow();

protected:
	/** Actual activation logic of the gameplay ability. */
	void DrawPrimaryItemInternal() const;
	/** Actual deactivation logic of the gameplay ability. */
	void SheathePrimaryItemInternal() const;
	/** Actual activation logic of the gameplay ability. */
	void StartItemThrowInternal();
	/** Actual deactivation logic of the gameplay ability. */
	void StopItemThrowInternal();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCharacterEquipmentComponent* CharacterEquipmentComponent;

	FDelegateHandle OnThrowItemDelegateHandle;
#pragma endregion

#pragma region AIMING

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingStateChanged, bool)
	FOnAimingStateChanged OnAimingEvent;

	/** Is the gameplay ability currently active? (activation flag replicated from server) */
	bool IsAiming() const;
	float GetCurrentAimingMovementSpeed() const { return CurrentAimingMovementSpeed; }
	/** Activates the ability via GAS. */
	void StartAiming();
	/** Deactivates the ability via GAS. */
	void StopAiming();
	UFUNCTION(BlueprintNativeEvent, Category = "Base Character")
	void OnStartAiming();
	UFUNCTION(BlueprintNativeEvent, Category = "Base Character")
	void OnStopAiming();

protected:
	/** Actual activation logic of the gameplay ability. */
	void StartAimingInternal();
	/** Actual deactivation logic of the gameplay ability. */
	void StopAimingInternal();
	virtual void OnStartAiming_Implementation() {}
	virtual void OnStopAiming_Implementation() {}

	float CurrentAimingMovementSpeed = 0.f;
#pragma endregion

#pragma region RANGED WEAPONS

public:
	/** Is the gameplay ability currently active? (activation flag replicated from server) */
	bool IsFiringWeapon() const;
	/** Activates the ability via GAS. */
	void StartWeaponFire();
	/** Deactivates the ability via GAS. */
	void StopWeaponFire();
	void ActivateNextWeaponMode() const;
	/** Is the gameplay ability currently active? (activation flag replicated from server) */
	bool IsReloadingWeapon() const;
	float GetCurrentReloadingWalkSpeed() const;
	/** Activates the ability via GAS. */
	void StartWeaponReload();
	/** Activates the ability via GAS. */
	void StartWeaponAutoReload();
	/** Deactivates the ability via GAS. */
	void StopWeaponReload();

protected:
	/** Actual activation logic of the gameplay ability. */
	void StartWeaponFireInternal();
	/** Actual deactivation logic of the gameplay ability. */
	void StopWeaponFireInternal() const;
	/** Actual activation logic of the gameplay ability. */
	void StartWeaponReloadInternal();
	/** Actual deactivation logic of the gameplay ability. */
	void StopWeaponReloadInternal() const;
	/** Triggers weapon fire after a delay. */
	void DelayWeaponFire(float DelayLength);

	/** Delay before the character can shoot. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character", meta = (ClampMin = 0.f, UIMin = 0.f))
	float DelayWeaponFireOnAiming = .15f;

	FTimerHandle DelayWeaponFireTimerHandle;
#pragma endregion

#pragma region MELEE WEAPONS

public:
	/** Activates the ability via GAS. */
	void StartPrimaryMeleeAttack();
	/** Deactivates the ability via GAS. */
	void StopPrimaryMeleeAttack();
	/** Activates the ability via GAS. */
	void StartSecondaryMeleeAttack();
	/** Deactivates the ability via GAS. */
	void StopSecondaryMeleeAttack();

protected:
	/** Actual activation logic of the gameplay ability. */
	void StartPrimaryMeleeAttackInternal();
	/** Actual deactivation logic of the gameplay ability. */
	void StopPrimaryMeleeAttackInternal();
	/** Actual activation logic of the gameplay ability. */
	void StartSecondaryMeleeAttackInternal();
	/** Actual deactivation logic of the gameplay ability. */
	void StopSecondaryMeleeAttackInternal();

	FDelegateHandle OnMeleeAttackActivatedDelegateHandle;
#pragma endregion
};
