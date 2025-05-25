// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "AbilitySystem/AttributeSets/XyzCharacterAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

void UXyzCharacterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UXyzCharacterAttributeSet, Health)
	DOREPLIFETIME(UXyzCharacterAttributeSet, MaxHealth)
	DOREPLIFETIME(UXyzCharacterAttributeSet, Defence)
	DOREPLIFETIME(UXyzCharacterAttributeSet, Stamina)
	DOREPLIFETIME(UXyzCharacterAttributeSet, MaxStamina)
	DOREPLIFETIME(UXyzCharacterAttributeSet, Oxygen)
	DOREPLIFETIME(UXyzCharacterAttributeSet, MaxOxygen)
}

void UXyzCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetHealthAttribute())
	{
		PreviousHealth = GetHealth();
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		PreviousStamina = GetStamina();
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
	else if (Attribute == GetOxygenAttribute())
	{
		PreviousOxygen = GetOxygen();
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxOxygen());
	}

	Super::PreAttributeChange(Attribute, NewValue);
}

void UXyzCharacterAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxHealth.GetBaseValue());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxStamina.GetBaseValue());
	}
	else if (Attribute == GetOxygenAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, MaxOxygen.GetBaseValue());
	}

	Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UXyzCharacterAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		if ((GetHealth() != 0.f || PreviousHealth != 0.f) && (GetHealth() != GetMaxHealth() || PreviousHealth != GetMaxHealth()))
		{
			OnHealthChanged(Data.EvaluatedData.Magnitude);
		}
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		if ((GetStamina() != 0.f || PreviousStamina != 0.f) && (GetStamina() != GetMaxStamina() || PreviousStamina != GetMaxStamina()))
		{
			OnStaminaChanged(Data.EvaluatedData.Magnitude);
		}
	}
	else if (Data.EvaluatedData.Attribute == GetOxygenAttribute())
	{
		if ((GetOxygen() != 0.f || PreviousOxygen != 0.f) && (GetOxygen() != GetMaxOxygen() || PreviousOxygen != GetMaxOxygen()))
		{
			OnOxygenChanged(Data.EvaluatedData.Magnitude);
		}
	}
}

void UXyzCharacterAttributeSet::AddHealth(float Amount)
{
	SetHealth(FMath::Clamp(GetHealth() + Amount, 0.f, GetMaxHealth()));
}

void UXyzCharacterAttributeSet::AddStamina(float Amount)
{
	SetStamina(FMath::Clamp(GetStamina() + Amount, 0.f, GetMaxStamina()));
}

void UXyzCharacterAttributeSet::AddOxygen(float Amount)
{
	SetOxygen(FMath::Clamp(GetOxygen() + Amount, 0.f, GetMaxOxygen()));
}

void UXyzCharacterAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UXyzCharacterAttributeSet, Health, OldValue);
	OnHealthChanged();
}

void UXyzCharacterAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UXyzCharacterAttributeSet, MaxHealth, OldValue);
	OnHealthChanged();
}

void UXyzCharacterAttributeSet::OnRep_Defence(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UXyzCharacterAttributeSet, Defence, OldValue);
}

void UXyzCharacterAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UXyzCharacterAttributeSet, Stamina, OldValue);
	OnStaminaChanged();
}

void UXyzCharacterAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UXyzCharacterAttributeSet, MaxStamina, OldValue);
	OnStaminaChanged();
}

void UXyzCharacterAttributeSet::OnRep_Oxygen(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UXyzCharacterAttributeSet, Oxygen, OldValue);
	OnOxygenChanged();
}

void UXyzCharacterAttributeSet::OnRep_MaxOxygen(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UXyzCharacterAttributeSet, MaxOxygen, OldValue);
	OnOxygenChanged();
}

void UXyzCharacterAttributeSet::OnHealthChanged(float Magnitude/* = 0.f*/) const
{
	if (OnHealthChangedEvent.IsBound())
	{
		OnHealthChangedEvent.Broadcast(GetHealth() / GetMaxHealth());
	}

	if (GetHealth() == 0.f)
	{
		OnDeath();
	}
}

void UXyzCharacterAttributeSet::OnDeath() const
{
	if (OnDeathEvent.IsBound())
	{
		OnDeathEvent.Broadcast(false);
	}
}

void UXyzCharacterAttributeSet::OnStaminaChanged(float Magnitude/* = 0.f*/) const
{
	if (OnStaminaChangedEvent.IsBound())
	{
		OnStaminaChangedEvent.Broadcast(GetStamina() / GetMaxStamina());
	}

	if (GetStamina() == 0.f)
	{
		OnOutOfStamina(true);
	}
	else if (GetStamina() == GetMaxStamina())
	{
		OnOutOfStamina(false);
	}
}

void UXyzCharacterAttributeSet::OnOutOfStamina(bool bIsOutOfStamina) const
{
	if (OnOutOfStaminaEvent.IsBound())
	{
		OnOutOfStaminaEvent.Broadcast(bIsOutOfStamina);
	}
}

void UXyzCharacterAttributeSet::OnOxygenChanged(float Magnitude/* = 0.f*/) const
{
	if (OnOxygenChangedEvent.IsBound())
	{
		OnOxygenChangedEvent.Broadcast(GetOxygen() / GetMaxOxygen());
	}

	if (GetOxygen() == 0.f)
	{
		OnOutOfOxygen(true);
	}
	else if (FMath::IsNearlyEqual(GetOxygen(), 1.f, Magnitude))
	{
		OnOutOfOxygen(false);
	}
}

void UXyzCharacterAttributeSet::OnOutOfOxygen(bool bIsOutOfOxygen) const
{
	if (OnOutOfOxygenEvent.IsBound())
	{
		OnOutOfOxygenEvent.Broadcast(bIsOutOfOxygen);
	}
}
