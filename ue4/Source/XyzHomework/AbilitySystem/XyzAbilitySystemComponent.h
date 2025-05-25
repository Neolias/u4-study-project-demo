// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "XyzGenericEnums.h"
#include "XyzAbilitySystemComponent.generated.h"

USTRUCT(BlueprintType)
struct FGameplayAbilityTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UGameplayAbility> AbilityClass;
};

USTRUCT(BlueprintType)
struct FGameplayEffectTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UGameplayEffect> EffectClass;
};

/** Extended component that provides an interface for interacting with GAS via EGameplayAbility and EGameplayEffect enums. Additionally, includes tools for tracking ability and effects statuses, both on the server and clients. */
UCLASS()
class XYZHOMEWORK_API UXyzAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FAbilityDeactivationFailureEvent, UGameplayAbility*)
	FAbilityDeactivationFailureEvent AbilityDeactivationFailureEvent;
	
	UXyzAbilitySystemComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CancelAbilitySpec(FGameplayAbilitySpec& Spec, UGameplayAbility* Ignore) override;
	/** Loads ability and effect data. Gives abilities to the owner. Activates startup abilities and applies startup effects to the owner. */
	void OnStartup();
	/** Returns an ability activation status replicated from the server. */
	bool IsAbilityActiveRemote(EGameplayAbility Ability) const;
	/** Returns an ability activation status stored on this client (can be different from the server). */
	bool IsAbilityActiveLocal(EGameplayAbility Ability) const;
	/** Returns an applied-to-self effect status replicated from the server. */
	bool IsEffectActiveRemote(EGameplayEffect Effect) const;
	/** Returns an applied-to-self effect status stored on this client (can be different from the server). */
	bool IsEffectActiveLocal(EGameplayEffect Effect) const;
	/** Activates/deactivates an ability.
	 * @param bIsActivated Desired status.
	 */
	void ActivateAbility(EGameplayAbility Ability, bool bIsActivated, bool bAllowRemoteActivation = true);
	/** Applies/removes an effect from the owner.
	* @param bIsApplied Desired status.
	 */
	void ApplyEffectToSelf(EGameplayEffect Effect, bool bIsApplied);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|GAS")
	TSoftObjectPtr<UDataTable> GameplayAbilityDataTable;
	/** Abilities that are given to this actor on startup. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|GAS")
	TArray<EGameplayAbility> AvailableAbilities;
	/** Abilities that are activated on startup. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|GAS")
	TArray<EGameplayAbility> StartupAbilities;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|GAS")
	TSoftObjectPtr<UDataTable> GameplayEffectDataTable;
	/** Effects that are applied to this actor on startup. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|GAS")
	TArray<EGameplayEffect> StartupEffectsAppliedToSelf;

private:
	TSubclassOf<UGameplayAbility> LoadAbilityClassFromDataTable(EGameplayAbility Ability) const;
	TSubclassOf<UGameplayEffect> LoadEffectClassFromDataTable(EGameplayEffect Effect) const;
	const FGameplayAbilitySpec* FindAbilitySpecFromClassConst(TSubclassOf<UGameplayAbility> InAbilityClass) const;
	const FActiveGameplayEffect* FindActiveEffectFromClass(TSubclassOf<UGameplayEffect> InEffectClass) const;
	/** Converts UGameplayAbility into an index used as an ID. Return 0 if the ability is missing from GameplayAbilityDataTable. */
	uint32 GetAbilityIndex(const UGameplayAbility* Ability) const;
	/** Updates the ability status flag stored in AbilityStatusFlags. */
	void OnAbilityStatusChanged(UGameplayAbility* Ability);
	/** Rolls back the activation/deactivation of a predicted ability if failed. */
	void OnAbilityActivationFailure(const UGameplayAbility* Ability, bool bFailedActivationFlag);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HandlePredictedAbilityActivationFailure(EGameplayAbility Ability, bool bFailedActivationFlag);
	void OnEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle);
	void OnActiveEffectRemoved(const FActiveGameplayEffect& ActiveGameplayEffect);
	/** Updates the effect status flag stored in EffectStatusFlags.
	 * @param bIsApplied New flag.
	 */
	void UpdateEffectStatus(const UGameplayEffect* Effect, bool bIsApplied);

	/** List of known ability subclasses loaded. */
	UPROPERTY()
	TArray<TSubclassOf<UGameplayAbility>> AvailableAbilityClasses;
	/** List of known effect subclasses loaded. */
	UPROPERTY()
	TArray<TSubclassOf<UGameplayEffect>> AvailableEffectClasses;
	/** List of ability status flags defined by the server and replicated to clients. */
	UPROPERTY(Replicated)
	TArray<bool> AbilityStatusFlags;
	/** List of effect status flags defined by the server and replicated to clients. */
	UPROPERTY(Replicated)
	TArray<bool> EffectStatusFlags;
	bool bIsInitialized = false;
};
