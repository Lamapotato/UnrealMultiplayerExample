// Copyright Sam Bonifacio. All Rights Reserved.

using UnrealBuildTool;

public class SBAutomationAutoSettingsEditor : ModuleRules
{
	public SBAutomationAutoSettingsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		if (Target.Type != TargetType.Editor)
		{
			throw new BuildException("Unable to instantiate SBAutomationAutoSettingsEditor module for non-editor targets.");
		}

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"AssetRegistry",
			"AutomationTest",
			"Core",
			"CoreUObject",
			// CQTest exposes FTestCommandBuilder through the shared future adapter interface.
			"CQTest"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			// CQTest hosts editor regression tests, while DirectoryWatcher lets cleanup settle package-file notifications.
			"CQTest",
			"DirectoryWatcher",
			"Engine",
			"LevelEditor",
			// Core's Clang-only Windows imports require exports absent from launcher-engine binaries.
			"SBClangAutoSettings",
			"UnrealEd"
		});
	}
}
