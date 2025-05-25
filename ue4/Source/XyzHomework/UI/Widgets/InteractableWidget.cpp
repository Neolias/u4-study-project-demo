// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "UI/Widgets/InteractableWidget.h"

#include "Components/TextBlock.h"

void UInteractableWidget::SetKeyName(FName KeyName) const
{
	if (KeyText)
	{
		KeyText->SetText(FText::FromName(KeyName));
	}
}
