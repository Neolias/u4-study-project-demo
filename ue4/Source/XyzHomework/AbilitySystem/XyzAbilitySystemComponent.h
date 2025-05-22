// Fill out your copyright notice in the Description page of Project Settings.

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

/**
 * 
 */
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
	void OnStartup();
	bool IsAbilityActiveRemote(EGameplayAbility Ability) const;
	bool IsAbilityActiveLocal(EGameplayAbility Ability) const;
	bool IsEffectActiveRemote(EGameplayEffect Effect) const;
	bool IsEffectActiveLocal(EGameplayEffect Effect) const;
	void ActivateAbility(EGameplayAbility Ability, bool bIsActivated, bool bAllowRemoteActivation = true);
	void ApplyEffectToSelf(EGameplayEffect Effect, bool bIsApplied);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|GAS")
	TSoftObjectPtr<UDataTable> GameplayAbilityDataTable;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|GAS")
	TArray<EGameplayAbility> AvailableAbilities;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|GAS")
	TArray<EGameplayAbility> StartupAbilities;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|GAS")
	TSoftObjectPtr<UDataTable> GameplayEffectDataTable;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|GAS")
	TArray<EGameplayEffect> StartupEffectsAppliedToSelf;

private:
	TSubclassOf<UGameplayAbility> LoadAbilityClassFromDataTable(EGameplayAbility Ability) const;
	TSubclassOf<UGameplayEffect> LoadEffectClassFromDataTable(EGameplayEffect Effect) const;
	const FGameplayAbilitySpec* FindAbilitySpecFromClassConst(TSubclassOf<UGameplayAbility> InAbilityClass) const;
	const FActiveGameplayEffect* FindActiveEffectFromClass(TSubclassOf<UGameplayEffect> InEffectClass) const;
	uint32 GetAbilityIndex(const UGameplayAbility* Ability) const;
	void OnAbilityStatusChanged(UGameplayAbility* Ability);
	void OnAbilityActivationFailure(const UGameplayAbility* Ability, bool bActivationFlag);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HandlePredictedAbilityActivationFailure(EGameplayAbility Ability, bool bFailedActivationFlag);
	void OnEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle);
	void OnActiveEffectRemoved(const FActiveGameplayEffect& ActiveGameplayEffect);
	void UpdateEffectStatus(const UGameplayEffect* Effect, bool bIsApplied);

	UPROPERTY()
	TArray<TSubclassOf<UGameplayAbility>> AvailableAbilityClasses;
	UPROPERTY()
	TArray<TSubclassOf<UGameplayEffect>> AvailableEffectClasses;
	UPROPERTY(Replicated)
	TArray<bool> AbilityStatusFlags;
	UPROPERTY(Replicated)
	TArray<bool> EffectStatusFlags;
	bool bIsInitialized = false;
};
