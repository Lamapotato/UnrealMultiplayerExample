// Copyright Sam Bonifacio. All Rights Reserved.

using UnrealBuildTool;

public class AutoSettingsCommonUI : ModuleRules
{
	public AutoSettingsCommonUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"AutoSettingsInput",
				"AutoSettings",
				"AutoSettingsUI"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UMG",
				"CommonInput",
				"InputCore",
				"AutoSettingsCore",
				"GameplayTags",
				// Each Unreal module DLL must link the launcher-engine Clang exports it imports from Core headers.
				"SBClangAutoSettings",
				"CommonUI",
				"DeveloperSettings"
			}
		);

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new[]
				{
					"ContentBrowser"
				}
			);
		}
	}
}