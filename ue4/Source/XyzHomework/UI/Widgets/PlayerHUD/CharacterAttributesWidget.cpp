// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

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
