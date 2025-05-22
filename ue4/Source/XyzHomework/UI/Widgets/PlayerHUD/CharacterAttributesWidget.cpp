// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAttributesWidget.h"

void UCharacterAttributesWidget::OnHealthChanged(float NewHealthPercentage)
{
	HealthPercentage = NewHealthPercentage;
}

void UCharacterAttributesWidget::OnStaminaChanged(float NewStaminaPercentage)
{
	StaminaPercentage = NewStaminaPercentage;
	bIsStaminaBarVisible = StaminaPercentage < 1.f;
}

void UCharacterAttributesWidget::OnOxygenChanged(float NewOxygenPercentage)
{
	OxygenPercentage = NewOxygenPercentage;
	bIsOxygenBarVisible = OxygenPercentage < 1.f;
}
