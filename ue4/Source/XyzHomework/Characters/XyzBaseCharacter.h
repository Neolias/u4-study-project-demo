// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "SignificanceManager.h"
#include "XyzGenericStructs.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "GameFramework/Character.h"
#include "Subsystems/SaveSubsystem/SaveSubsystemInterface.h"
#include "XyzBaseCharacter.generated.h"

class UInventorySlot;
class UInventoryItem;
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
class UWidgetComponent;
class UXyzAbilitySystemComponent;
class UXyzBaseCharMovementComponent;
class UXyzCharacterAttributeSet;

typedef TArray<AEnvironmentActor*, TInlineAllocator<10>> TInteractiveActorsArray;

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

/**
 *
 */
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character")
	ETeam Team = ETeam::Enemy;

	TWeakObjectPtr<AXyzPlayerController> CachedPlayerController;
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
	void SetGameplayAbilityActivationFlag(EGameplayAbility Ability, EGameplayAbilityActivationFlag ActivationFlag);

	// IAbilitySystemInterface	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ IAbilitySystemInterface

protected:
	void InitializeGameplayAbilityCallbacks();
	void TryActivateGameplayAbilitiesWithFlags();
	void ExecuteGameplayAbilityCallbackInternal(EGameplayAbility Ability, bool bIsActivationCallback, bool bIsServerCall = true);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExecuteGameplayAbilityCallback(EGameplayAbility Ability, bool bIsActivationCallback);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UXyzAbilitySystemComponent* AbilitySystemComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UXyzCharacterAttributeSet* AttributeSet;

	TArray<EGameplayAbilityActivationFlag> AbilityActivationFlags;
	TArray<TFunction<void()>> AbilityActivationCallbacks;
	TArray<TFunction<void()>> AbilityDeactivationCallbacks;
#pragma endregion

#pragma region INVENTORY

public:
	UCharacterInventoryComponent* GetCharacterInventoryComponent() const { return CharacterInventoryComponent; }
	void TogglePlayerMouseInput(APlayerController* PlayerController);
	void UpdatePlayerMouseInput(APlayerController* PlayerController) const;
	void UseInventory(APlayerController* PlayerController) const;
	void UseRadialMenu(APlayerController* PlayerController) const;
	bool PickupItem(EInventoryItemType ItemType, int32 Amount) const;
	void DropItem(EInventoryItemType ItemType, int32 Amount) const;
	UFUNCTION(Server, Reliable)
	void Server_UseItem(UInventoryItem* InventoryItem, UInventorySlot* InventorySlot = nullptr);
	UFUNCTION(Server, Reliable)
	void Server_AddEquipmentItem(EInventoryItemType ItemType, int32 Amount = 1, int32 EquipmentSlotIndex = -1);
	UFUNCTION(Server, Reliable)
	void Server_RemoveEquipmentItem(int32 EquipmentSlotIndex);
	UFUNCTION(Server, Reliable)
	void Server_DropItem(EInventoryItemType ItemType, int32 Amount);
	bool AddAmmoToInventory(EWeaponAmmoType AmmoType, int32 Amount) const;
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

	void InteractWithObject();

protected:
	UFUNCTION(Server, Reliable)
	void Server_InteractWithObject(UObject* InteractableObject);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_InteractWithObject(UObject* InteractableObject);
	void LineTraceInteractableObject();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character", meta = (ClampMin = 1.f, UIMin = 1.f))
	float InteractiveObjectRange = 500.f;

	TScriptInterface<IInteractable> CurrentInteractableObject;
#pragma endregion

#pragma region INVERSE KINEMATICS

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Character|Inverse Kinematics")
	float GetIKLeftFootOffset() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Character|Inverse Kinematics")
	float GetIKRightFootOffset() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Character|Inverse Kinematics")
	float GetIKPelvisOffset() const;

protected:
	void UpdateIKSettings(float DeltaSeconds);
	float GetIKOffsetForFootSocket(FName SocketName) const;
	float GetIKOffsetForPelvisSocket() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Inverse Kinematics")
	FName LeftFootSocketName = "foot_l_socket";
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Inverse Kinematics")
	FName RightFootSocketName = "foot_r_socket";
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Inverse Kinematics", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxFeetIKOffset = 50.f;
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
	virtual void StartJump();
	virtual void MoveForward(float Value) {}
	virtual void MoveRight(float Value) {}
	virtual void Turn(float Value) {}
	virtual void LookUp(float Value) {}
	virtual void TurnAtRate(float Value) {}
	virtual void LookUpAtRate(float Value) {}

protected:
	void StartJumpInternal();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character|Movement", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BaseTurnRate = 45.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character|Movement", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BaseLookUpRate = 45.f;

	UPROPERTY()
	UXyzBaseCharMovementComponent* BaseCharacterMovementComponent;
#pragma endregion

#pragma region SWIMMING

public:
	virtual void SwimForward(float Value) {}
	virtual void SwimRight(float Value) {}
	virtual void SwimUp(float Value) {}
	bool IsDiveAbilityActive() const;
	void StartDive();
	void StopDive();
	bool IsDiving() const;
	void OnDiving(bool bIsDiving) const;

protected:
	void StartDiveInternal();
	void StopDiveInternal() const;

	UPROPERTY(EditAnywhere, Category = "Base Character|Movement", meta = (ClampMin = 0.f, UIMin = 0.f))
	float DiveAbilityLength = .5f;

	FTimerHandle DiveTimerHandle;
#pragma endregion

#pragma region SPRINTING

public:
	bool IsSprinting() const;
	virtual void StartSprint();
	void StopSprint();
	UFUNCTION(BlueprintNativeEvent, Category = "Base Character|Movement")
	void OnStartSprint();
	UFUNCTION(BlueprintNativeEvent, Category = "Base Character|Movement")
	void OnStopSprint();

protected:
	void StartSprintInternal();
	void StopSprintInternal() const;
	void OnStartSprint_Implementation();
	void OnStopSprint_Implementation();
#pragma endregion

#pragma region SLIDING

public:
	bool IsSliding() const;
	void StartSlide();
	void StopSlide();
	virtual void OnStartSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	virtual void OnStopSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

protected:
	void StartSlideInternal() const;
	void StopSlideInternal();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character|Movement")
	UAnimMontage* SlideAnimMontage;
#pragma endregion

#pragma region CROUCHING

public:
	bool IsCrouching() const;
	virtual bool CanChangeCrouchState() const { return true; }
	virtual void Crouch(bool bClientSimulation = false) override;
	virtual void UnCrouch(bool bClientSimulation = false) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

protected:
	void CrouchInternal() const;
	void UnCrouchInternal() const;
#pragma endregion

#pragma region PRONE

public:
	virtual bool IsProne() const;
	virtual bool CanChangeProneState() const { return true; }
	void Prone();
	void UnProne();
	void ToggleProne();
	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	virtual void OnStopProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

protected:
	void ProneInternal() const;
	void UnProneInternal() const;

	float ProneEyeHeight = 0.f;
#pragma endregion

#pragma region MANTLING

public:
	void StartMantle();
	void StopMantle();
	bool DetectLedge(OUT FLedgeDescription& LedgeDescription) const;
	UFUNCTION(BlueprintCallable, Category = "Base Character|Movement")
	void Mantle(bool bForceMantle = false);

protected:
	const FMantlingSettings& GetMantlingSettings(float LedgeHeight) const;
	void StartMantleInternal();
	virtual void OnMantle(const FMantlingSettings& MantlingSettings, const FMantlingMovementParameters& MantlingParameters) {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Movement|Mantling", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MantlingDepth = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Movement|Mantling", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ForwardTraceDistance = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Movement|Mantling")
	FMantlingSettings HighMantleSettings;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Movement|Mantling")
	FMantlingSettings LowMantleSettings;
#pragma endregion

#pragma region ENVIRONMENT ACTORS

public:
	void RegisterEnvironmentActor(AEnvironmentActor* EnvironmentActor);
	void UnRegisterEnvironmentActor(AEnvironmentActor* EnvironmentActor);
	bool IsUsingEnvironmentActor() const;
	bool CanUseEnvironmentActors() const;
	void ToggleUseEnvironmentActor();
	void StartUseEnvironmentActor();
	void StopUseEnvironmentActor();
	bool TryAttachCharacterToZipline();
	bool TryAttachCharacterToLadder();
	virtual void ClimbLadderUp(float Value) {}

protected:
	ALadder* GetAvailableLadder();
	AZipline* GetAvailableZipline();
	void StartUseEnvironmentActorInternal();
	void StopUseEnvironmentActorInternal() const;
	virtual void OnAttachedToLadderFromTop(ALadder* Ladder) {}

	TInteractiveActorsArray EnvironmentActors;
#pragma endregion

#pragma region WALL RUNNING

public:
	virtual void OnWallRunStart() {}
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
	float LandEquipItemTimerLength = .2f;
#pragma endregion

#pragma region ATTRIBUTES

public:
	UWidgetComponent* GetCharacterWidgetComponent() const { return WidgetComponent; }
	UFUNCTION(BlueprintCallable, Category = "Base Character")
	bool IsDead() const;
	bool IsOutOfStamina() const;

protected:
	virtual void StartOutOfStaminaInternal();
	virtual void StopOutOfStaminaInternal();
	UFUNCTION()
	void OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	virtual void OnDeath(bool bShouldPlayAnimMontage);
	void OnOutOfStamina(bool bIsOutOfStamina) const;
	void OnOutOfOxygen(bool bIsOutOfOxygen) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* WidgetComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character")
	UAnimMontage* DeathAnimMontage;
#pragma endregion

#pragma region EQUIPMENT

public:
	UCharacterEquipmentComponent* GetCharacterEquipmentComponent() const { return CharacterEquipmentComponent; }
	void DrawNextItem();
	void DrawPreviousItem();
	void EquipItemFromCurrentSlot(bool bShouldSkipAnimation = false);
	bool IsPrimaryItemDrawn() const;
	float GetCurrentThrowItemMovementSpeed() const;
	void TogglePrimaryItem();
	void SheathePrimaryItem();
	bool IsThrowingItem() const;
	void StartItemThrow();
	void StopItemThrow();

protected:
	void DrawPrimaryItemInternal() const;
	void SheathePrimaryItemInternal() const;
	void StartItemThrowInternal();
	void StopItemThrowInternal();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCharacterEquipmentComponent* CharacterEquipmentComponent;

	FDelegateHandle OnThrowItemDelegateHandle;
#pragma endregion

#pragma region AIMING

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingStateChanged, bool)
	FOnAimingStateChanged OnAimingEvent;

	bool IsAiming() const;
	float GetCurrentAimingMovementSpeed() const { return CurrentAimingMovementSpeed; }
	void StartAiming();
	void StopAiming();
	UFUNCTION(BlueprintNativeEvent, Category = "Base Character")
	void OnStartAiming();
	UFUNCTION(BlueprintNativeEvent, Category = "Base Character")
	void OnStopAiming();

protected:
	void StartAimingInternal();
	void StopAimingInternal();
	virtual void OnStartAiming_Implementation() {}
	virtual void OnStopAiming_Implementation() {}

	float CurrentAimingMovementSpeed = 0.f;
#pragma endregion

#pragma region RANGED WEAPONS

public:
	bool IsFiringWeapon() const;
	void StartWeaponFire();
	void StopWeaponFire();
	void ActivateNextWeaponMode() const;
	bool IsReloadingWeapon() const;
	float GetCurrentReloadingWalkSpeed() const;
	void StartWeaponReload();
	void StartWeaponAutoReload();
	void StopWeaponReload();

protected:
	void StartWeaponFireInternal();
	void StopWeaponFireInternal() const;
	void StartWeaponReloadInternal();
	void StopWeaponReloadInternal() const;
	void DelayWeaponFire(float DelayLength);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character", meta = (ClampMin = 0.f, UIMin = 0.f))
	float DelayWeaponFireOnAiming = .15f;

	FTimerHandle DelayWeaponFireTimerHandle;
#pragma endregion

#pragma region MELEE WEAPONS

public:
	void StartPrimaryMeleeAttack();
	void StopPrimaryMeleeAttack();
	void StartSecondaryMeleeAttack();
	void StopSecondaryMeleeAttack();

protected:
	void StartPrimaryMeleeAttackInternal();
	void StopPrimaryMeleeAttackInternal();
	void StartSecondaryMeleeAttackInternal();
	void StopSecondaryMeleeAttackInternal();

	FDelegateHandle OnMeleeAttackActivatedDelegateHandle;
#pragma endregion
};
