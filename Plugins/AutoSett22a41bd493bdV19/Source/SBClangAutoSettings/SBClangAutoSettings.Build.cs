// Copyright Sam Bonifacio. All Rights Reserved.

using UnrealBuildTool;

public class SBClangAutoSettings : ModuleRules
{
	public SBClangAutoSettings(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.Add("Core");
	}
}
