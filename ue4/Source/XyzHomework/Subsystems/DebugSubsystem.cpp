// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "Subsystems/DebugSubsystem.h"

bool UDebugSubsystem::IsCategoryEnabled(const FName& CategoryName) const
{
    const bool* bIsEnabled = EnabledDebugCategories.Find(CategoryName);
    return bIsEnabled != nullptr && *bIsEnabled;
}

void UDebugSubsystem::EnableDebugCategory(const FName& CategoryName, const bool bIsEnabled)
{
    EnabledDebugCategories.FindOrAdd(CategoryName);
    EnabledDebugCategories[CategoryName] = bIsEnabled;
}