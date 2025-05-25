// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "AT_TickTaskBase.h"
#include "AT_UpdateSlide.generated.h"

UCLASS()
class XYZHOMEWORK_API UAT_UpdateSlide : public UAT_TickTaskBase
{
	GENERATED_BODY()

public:
	virtual void TickTask(float DeltaTime) override;
	static UAT_UpdateSlide* NewUpdateSlideTask(UGameplayAbility* OwningAbility);
};
