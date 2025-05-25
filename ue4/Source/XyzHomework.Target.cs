// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

using UnrealBuildTool;
using System.Collections.Generic;

public class XyzHomeworkTarget : TargetRules
{
	public XyzHomeworkTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "XyzHomework" } );
	}
}
