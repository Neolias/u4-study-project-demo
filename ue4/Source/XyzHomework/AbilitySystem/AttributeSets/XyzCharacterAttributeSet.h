// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "XyzCharacterAttributeSet.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

#define XYZ_GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName, OnPropertyChangedFuncName) \
FORCEINLINE void Set##PropertyName(float NewVal) \
{ \
UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent(); \
if (ensure(AbilityComp)) \
{ \
AbilityComp->SetNumericAttributeBase(Get##PropertyName##Attribute(), NewVal); \
}; \
##OnPropertyChangedFuncName##(NewVal - Get##PropertyName##()); \
}

#define XYZ_ATTRIBUTE_ACCESSORS(ClassName, PropertyName, OnPropertyChangedFuncName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
XYZ_GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName, OnPropertyChangedFuncName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class XYZHOMEWORK_API UXyzCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedEvent, float)
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAttributeTriggerEvent, bool)
	FOnAttributeChangedEvent OnHealthChangedEvent;
	FOnAttributeTriggerEvent OnDeathEvent;
	FOnAttributeChangedEvent OnStaminaChangedEvent;
	FOnAttributeTriggerEvent OnOutOfStaminaEvent;
	FOnAttributeChangedEvent OnOxygenChangedEvent;
	FOnAttributeTriggerEvent OnOutOfOxygenEvent;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	void AddHealth(float Amount);
	void AddStamina(float Amount);
	void AddOxygen(float Amount);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health, Category = "Base Character|Attribute Set", meta = (Hidden))
	FGameplayAttributeData Health = 200.f;
	XYZ_ATTRIBUTE_ACCESSORS(UXyzCharacterAttributeSet, Health, OnHealthChanged)
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHealth, Category = "Base Character|Attribute Set", meta = (Hidden))
	FGameplayAttributeData MaxHealth = 200.f;
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	ATTRIBUTE_ACCESSORS(UXyzCharacterAttributeSet, MaxHealth)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Defence, Category = "Base Character|Attribute Set", meta = (Hidden))
	FGameplayAttributeData Defence = 5.f;
	ATTRIBUTE_ACCESSORS(UXyzCharacterAttributeSet, Defence)
	UFUNCTION()
	void OnRep_Defence(const FGameplayAttributeData& OldValue);
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Stamina, Category = "Base Character|Attribute Set", meta = (Hidden))
	FGameplayAttributeData Stamina = 100.f;
	XYZ_ATTRIBUTE_ACCESSORS(UXyzCharacterAttributeSet, Stamina, OnStaminaChanged)
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxStamina, Category = "Base Character|Attribute Set", meta = (Hidden))
	FGameplayAttributeData MaxStamina = 100.f;
	ATTRIBUTE_ACCESSORS(UXyzCharacterAttributeSet, MaxStamina)
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Oxygen, Category = "Base Character|Attribute Set", meta = (Hidden))
	FGameplayAttributeData Oxygen = 100.f;
	XYZ_ATTRIBUTE_ACCESSORS(UXyzCharacterAttributeSet, Oxygen, OnOxygenChanged)
	UFUNCTION()
	void OnRep_Oxygen(const FGameplayAttributeData& OldValue);
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxOxygen, Category = "Base Character|Attribute Set", meta = (Hidden))
	FGameplayAttributeData MaxOxygen = 100.f;
	ATTRIBUTE_ACCESSORS(UXyzCharacterAttributeSet, MaxOxygen)
	UFUNCTION()
	void OnRep_MaxOxygen(const FGameplayAttributeData& OldValue);

private:
	void OnHealthChanged(float Magnitude = 0.f) const;
	void OnDeath() const;
	void OnStaminaChanged(float Magnitude = 0.f) const;
	void OnOutOfStamina(bool bIsOutOfStamina) const;
	void OnOxygenChanged(float Magnitude = 0.f) const;
	void OnOutOfOxygen(bool bIsOutOfOxygen) const;

	float AttributeComparisonTolerance = 0.5f;
	float PreviousHealth = 0.f;
	float PreviousStamina = 0.f;
	float PreviousOxygen = 0.f;
};
