// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

using UnrealBuildTool;
using System.Collections.Generic;

public class XyzHomeworkEditorTarget : TargetRules
{
	public XyzHomeworkEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "XyzHomework" } );
	}
}
