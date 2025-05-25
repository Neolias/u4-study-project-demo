// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "UI/Widgets/World/CharacterProgressBarWidget.h"

#include "Components/ProgressBar.h"

void UCharacterProgressBarWidget::SetHealthProgressBar(float Percentage) const
{
	HealthProgressBar->SetPercent(Percentage);
}
