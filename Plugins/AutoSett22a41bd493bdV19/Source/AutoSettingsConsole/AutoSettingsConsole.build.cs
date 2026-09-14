// Copyright Sam Bonifacio. All Rights Reserved.

using UnrealBuildTool;

public class AutoSettingsConsole : ModuleRules
{
	public AutoSettingsConsole(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"CoreUObject",
				"Engine",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"AutoSettingsCore",
				"AssetRegistry",
				// Each Unreal module DLL must link the launcher-engine Clang exports it imports from Core headers.
				"SBClangAutoSettings"
			}
		);
	}
}
