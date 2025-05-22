// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"

#include "CharacterAttributesWidget.h"
#include "ReticleWidget.h"
#include "WeaponAmmoWidget.h"
#include "Blueprint/WidgetTree.h"
#include "UI/Widgets/InteractableWidget.h"

UReticleWidget* UPlayerHUDWidget::GetReticleWidget() const
{
	return WidgetTree->FindWidget<UReticleWidget>(ReticleWidgetName);
}

UWeaponAmmoWidget* UPlayerHUDWidget::GetWeaponAmmoWidget() const
{
	return WidgetTree->FindWidget<UWeaponAmmoWidget>(WeaponAmmoWidgetName);
}

UCharacterAttributesWidget* UPlayerHUDWidget::GetCharacterAttributesWidget() const
{
	return WidgetTree->FindWidget<UCharacterAttributesWidget>(CharacterAttributesWidgetName);
}

UCharacterAttributesWidget* UPlayerHUDWidget::GetCharacterAttributesCenterWidget() const
{
	return WidgetTree->FindWidget<UCharacterAttributesWidget>(CharacterAttributesCenterWidgetName);
}

void UPlayerHUDWidget::SetInteractableKeyText(FName KeyName) const
{
	if (InteractableWidget)
	{
		InteractableWidget->SetKeyName(KeyName);
	}
}

void UPlayerHUDWidget::ShowInteractableKey(bool bIsVisible) const
{
	if (!InteractableWidget)
	{
		return;
	}

	if (bIsVisible)
	{
		InteractableWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		InteractableWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

