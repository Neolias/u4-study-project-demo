// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class XyzHomework : ModuleRules
{
	public XyzHomework(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"Niagara", 
			"UMG",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "SignificanceManager",
            "GameplayTasks",
            "GameplayAbilities",
            "GameplayTags"
        });

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");

        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		PrivateIncludePaths.AddRange(new string[] { Name });

		// OptimizeCode = CodeOptimization.Never;
		
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
