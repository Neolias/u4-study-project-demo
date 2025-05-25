// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DebugSubsystem.generated.h"

UCLASS()
class XYZHOMEWORK_API UDebugSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	bool IsCategoryEnabled(const FName& CategoryName) const;

private:
	UFUNCTION(Exec)
	void EnableDebugCategory(const FName& CategoryName, bool bIsEnabled);

	TMap<FName, bool> EnabledDebugCategories;
};
