// Copyright Sam Bonifacio. All Rights Reserved.

using UnrealBuildTool;

public class AutoSettingsUI : ModuleRules
{
	public AutoSettingsUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"Slate",
			"SlateCore",
			"GameplayTags",
			"AutoSettings",
			"AutoSettingsCore"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"AutoSettingsConsole",
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