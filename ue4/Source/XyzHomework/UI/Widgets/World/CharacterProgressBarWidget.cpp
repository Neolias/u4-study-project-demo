// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/World/CharacterProgressBarWidget.h"

#include "Components/ProgressBar.h"

void UCharacterProgressBarWidget::SetHealthProgressBar(float Percentage) const
{
	HealthProgressBar->SetPercent(Percentage);
}
