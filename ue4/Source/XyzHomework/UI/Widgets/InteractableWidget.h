// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractableWidget.generated.h"

UCLASS()
class XYZHOMEWORK_API UInteractableWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/** Updates the 'interaction key' text of the widget. */
	void SetKeyName(FName KeyName) const;

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* KeyText;	
};
