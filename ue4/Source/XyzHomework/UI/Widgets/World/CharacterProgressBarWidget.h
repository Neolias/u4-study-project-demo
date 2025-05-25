// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterProgressBarWidget.generated.h"

UCLASS()
class XYZHOMEWORK_API UCharacterProgressBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthProgressBar(float Percentage) const;

protected:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthProgressBar;	
};
