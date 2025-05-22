// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/XyzAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "XyzGenericEnums.h"
#include "Net/UnrealNetwork.h"

UXyzAbilitySystemComponent::UXyzAbilitySystemComponent()
{
	ReplicationMode = EGameplayEffectReplicationMode::Mixed;

	AvailableAbilityClasses.AddZeroed((uint32)EGameplayAbility::Max);
	AvailableEffectClasses.AddZeroed((uint32)EGameplayEffect::Max);
	AbilityStatusFlags.AddZeroed((uint32)EGameplayAbility::Max);
	EffectStatusFlags.AddZeroed((uint32)EGameplayEffect::Max);
}

void UXyzAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UXyzAbilitySystemComponent, AbilityStatusFlags)
	DOREPLIFETIME(UXyzAbilitySystemComponent, EffectStatusFlags)
}

void UXyzAbilitySystemComponent::CancelAbilitySpec(FGameplayAbilitySpec& Spec, UGameplayAbility* Ignore)
{
	Super::CancelAbilitySpec(Spec, Ignore);

	if (AbilityDeactivationFailureEvent.IsBound() && !Spec.Ability->CanBeCanceled())
	{
		AbilityDeactivationFailureEvent.Broadcast(Spec.Ability);
	}
}

void UXyzAbilitySystemComponent::OnStartup()
{
	if (!bIsInitialized)
	{
		for (uint32 Ability = 1; Ability < (uint32)EGameplayAbility::Max; ++Ability)
		{
			AvailableAbilityClasses[Ability] = LoadAbilityClassFromDataTable((EGameplayAbility)Ability);
		}

		for (uint32 Effect = 1; Effect < (uint32)EGameplayEffect::Max; ++Effect)
		{
			AvailableEffectClasses[Effect] = LoadEffectClassFromDataTable((EGameplayEffect)Effect);
		}

		if (IsValid(GetAvatarActor()) && GetAvatarActor()->GetLocalRole() == ROLE_Authority)
		{
			AbilityActivatedCallbacks.AddUObject(this, &UXyzAbilitySystemComponent::OnAbilityStatusChanged);
			AbilityEndedCallbacks.AddUObject(this, &UXyzAbilitySystemComponent::OnAbilityStatusChanged);
			AbilityFailedCallbacks.AddLambda([this](const UGameplayAbility* Ability, const FGameplayTagContainer& TagContainer)
			{
				OnAbilityActivationFailure(Ability, true);
			});
			AbilityDeactivationFailureEvent.AddLambda([this](const UGameplayAbility* Ability)
			{
				OnAbilityActivationFailure(Ability, false);
			});
			OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UXyzAbilitySystemComponent::OnEffectAppliedToSelf);
			ActiveGameplayEffects.OnActiveGameplayEffectRemovedDelegate.AddUObject(this, &UXyzAbilitySystemComponent::OnActiveEffectRemoved);

			for (EGameplayAbility Ability : AvailableAbilities)
			{
				if (TSubclassOf<UGameplayAbility> AbilityClass = AvailableAbilityClasses[(uint32)Ability])
				{
					GiveAbility(FGameplayAbilitySpec(AbilityClass));
				}
			}

			for (EGameplayAbility Ability : StartupAbilities)
			{
				ActivateAbility(Ability, true);
			}

			for (EGameplayEffect Effect : StartupEffectsAppliedToSelf)
			{
				ApplyEffectToSelf(Effect, true);
			}
		}

		bIsInitialized = true;
	}
}

bool UXyzAbilitySystemComponent::IsAbilityActiveRemote(EGameplayAbility Ability) const
{
	return AbilityStatusFlags[(uint32)Ability];
}

bool UXyzAbilitySystemComponent::IsAbilityActiveLocal(EGameplayAbility Ability) const
{
	const FGameplayAbilitySpec* Spec = FindAbilitySpecFromClassConst(AvailableAbilityClasses[(uint32)Ability]);
	return Spec && Spec->IsActive();
}

bool UXyzAbilitySystemComponent::IsEffectActiveRemote(EGameplayEffect Effect) const
{
	return EffectStatusFlags[(uint32)Effect];
}

bool UXyzAbilitySystemComponent::IsEffectActiveLocal(EGameplayEffect Effect) const
{
	const FActiveGameplayEffect* ActiveEffect = FindActiveEffectFromClass(AvailableEffectClasses[(uint32)Effect]);
	return ActiveEffect && ActiveEffect->Spec.Def;
}

void UXyzAbilitySystemComponent::ActivateAbility(EGameplayAbility Ability, bool bIsActivated, bool bAllowRemoteActivation/* = true*/)
{
	if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromClass(AvailableAbilityClasses[(uint32)Ability]))
	{
		if (bIsActivated)
		{
			TryActivateAbility(Spec->Handle, bAllowRemoteActivation);
		}
		else if (IsAbilityActiveLocal(Ability))
		{
			CancelAbilitySpec(*Spec, nullptr);
		}
	}
}

void UXyzAbilitySystemComponent::ApplyEffectToSelf(EGameplayEffect Effect, bool bIsApplied)
{
	auto EffectClass = AvailableEffectClasses[(uint32)Effect];
	if (bIsApplied)
	{
		if (EffectClass)
		{
			const UGameplayEffect* GameplayEffect = EffectClass->GetDefaultObject<UGameplayEffect>();
			FGameplayEffectSpec EffectSpec(GameplayEffect, FGameplayEffectContextHandle(new FGameplayEffectContext(GetOwnerActor(), GetAvatarActor())));
			ApplyGameplayEffectSpecToSelf(EffectSpec);
		}
	}
	else
	{
		RemoveActiveGameplayEffectBySourceEffect(EffectClass, nullptr);
	}
}

TSubclassOf<UGameplayAbility> UXyzAbilitySystemComponent::LoadAbilityClassFromDataTable(EGameplayAbility Ability) const
{
	const UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *GameplayAbilityDataTable.GetUniqueID().GetAssetPathString());
	checkf(DataTable != nullptr, TEXT("UXyzAbilitySystemComponent::LoadAbilityClassFromDataTable(): GameplayAbilityDataTable is undefined."));

	FString RowID = UEnum::GetDisplayValueAsText<EGameplayAbility>(Ability).ToString();
	if (const FGameplayAbilityTableRow* AbilityData = DataTable->FindRow<FGameplayAbilityTableRow>(FName(RowID), TEXT("Find gameplay ability data")))
	{
		return AbilityData->AbilityClass.LoadSynchronous();
	}

	return nullptr;
}

TSubclassOf<UGameplayEffect> UXyzAbilitySystemComponent::LoadEffectClassFromDataTable(EGameplayEffect Effect) const
{
	const UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *GameplayEffectDataTable.GetUniqueID().GetAssetPathString());
	checkf(DataTable != nullptr, TEXT("UXyzAbilitySystemComponent::LoadEffectClassFromDataTable(): GameplayEffectDataTable is undefined."));

	FString RowID = UEnum::GetDisplayValueAsText<EGameplayEffect>(Effect).ToString();
	if (const FGameplayEffectTableRow* EffectData = DataTable->FindRow<FGameplayEffectTableRow>(FName(RowID), TEXT("Find gameplay effect data")))
	{
		return EffectData->EffectClass.LoadSynchronous();
	}

	return nullptr;
}

const FGameplayAbilitySpec* UXyzAbilitySystemComponent::FindAbilitySpecFromClassConst(TSubclassOf<UGameplayAbility> InAbilityClass) const
{
	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability->GetClass() == InAbilityClass)
		{
			return &Spec;
		}
	}

	return nullptr;
}

const FActiveGameplayEffect* UXyzAbilitySystemComponent::FindActiveEffectFromClass(TSubclassOf<UGameplayEffect> InEffectClass) const
{
	FGameplayEffectQuery Query;
	Query.CustomMatchDelegate.BindLambda([&](const FActiveGameplayEffect& CurEffect)
	{
		return CurEffect.Spec.Def && InEffectClass == CurEffect.Spec.Def->GetClass();
	});

	auto Result = ActiveGameplayEffects.GetActiveEffects(Query);
	if (Result.Num() > 0)
	{
		return GetActiveGameplayEffect(Result[0]);
	}

	return nullptr;
}

uint32 UXyzAbilitySystemComponent::GetAbilityIndex(const UGameplayAbility* Ability) const
{
	if (!Ability)
	{
		return 0;
	}

	return AvailableAbilityClasses.IndexOfByPredicate([Ability](TSubclassOf<UGameplayAbility> AbilityClass)
	{
		return AbilityClass && AbilityClass == Ability->GetClass();
	});
}

void UXyzAbilitySystemComponent::OnAbilityStatusChanged(UGameplayAbility* Ability)
{
	if (Ability && Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
	{
		int32 Index = GetAbilityIndex(Ability);
		if (Index > 0 && Index < AvailableAbilityClasses.Num())
		{
			AbilityStatusFlags[Index] = Ability->IsActive();
		}
	}
}

void UXyzAbilitySystemComponent::OnAbilityActivationFailure(const UGameplayAbility* Ability, bool bActivationFlag)
{
	bool bIsLocalPredicted = Ability && Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced
		&& Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted && Ability->GetReplicationPolicy() == EGameplayAbilityReplicationPolicy::ReplicateYes;
	if (bIsLocalPredicted)
	{
		Multicast_HandlePredictedAbilityActivationFailure((EGameplayAbility)GetAbilityIndex(Ability), bActivationFlag);
	}
}

void UXyzAbilitySystemComponent::Multicast_HandlePredictedAbilityActivationFailure_Implementation(EGameplayAbility Ability, bool bFailedActivationFlag)
{
	// Server is already in the correct state
	if (!IsValid(GetAvatarActor()) || GetAvatarActor()->GetLocalRole() == ROLE_Authority)
	{
		return;
	}

	if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromClass(AvailableAbilityClasses[(uint32)Ability]))
	{
		if (bFailedActivationFlag)
		{
			CancelAbilitySpec(*Spec, nullptr);
		}
		else
		{
			TryActivateAbility(Spec->Handle, false);
		}
	}
}

void UXyzAbilitySystemComponent::OnEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	UpdateEffectStatus(GameplayEffectSpec.Def, true);
}

void UXyzAbilitySystemComponent::OnActiveEffectRemoved(const FActiveGameplayEffect& ActiveGameplayEffect)
{
	UpdateEffectStatus(ActiveGameplayEffect.Spec.Def, false);
}

void UXyzAbilitySystemComponent::UpdateEffectStatus(const UGameplayEffect* Effect, bool bIsApplied)
{
	if (Effect)
	{
		int32 Index = AvailableEffectClasses.IndexOfByPredicate([Effect](TSubclassOf<UGameplayEffect> EffectClass)
		{
			return EffectClass && EffectClass == Effect->GetClass();
		});

		if (Index > 0 && Index < AvailableEffectClasses.Num())
		{
			EffectStatusFlags[Index] = bIsApplied;
		}
	}
}
