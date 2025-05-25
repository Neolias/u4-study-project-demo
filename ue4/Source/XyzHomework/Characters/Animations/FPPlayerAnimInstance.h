// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "XyzBaseCharacterAnimInstance.h"
#include "FPPlayerAnimInstance.generated.h"

/** DEPRECATED */
UCLASS()
class XYZHOMEWORK_API UFPPlayerAnimInstance : public UXyzBaseCharacterAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;

private:
	TWeakObjectPtr<class AFPPlayerCharacter> CachedFPCharacter;
};
