// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/InteractableWidget.h"

#include "Components/TextBlock.h"

void UInteractableWidget::SetKeyName(FName KeyName) const
{
	if (KeyText)
	{
		KeyText->SetText(FText::FromName(KeyName));
	}
}
