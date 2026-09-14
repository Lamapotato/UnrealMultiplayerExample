// Copyright Sam Bonifacio. All Rights Reserved.

using UnrealBuildTool;

public class AutoSettings : ModuleRules
{
	public AutoSettings(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"GameplayTags",
			"DeveloperSettings",
			"AutoSettingsCore"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"AutoSettingsConsole",
			"Projects",
			// Each Unreal module DLL must link the launcher-engine Clang exports it imports from Core headers.
			"SBClangAutoSettings"
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new[]
			{
				"UnrealEd"
			});
		}
	}
}