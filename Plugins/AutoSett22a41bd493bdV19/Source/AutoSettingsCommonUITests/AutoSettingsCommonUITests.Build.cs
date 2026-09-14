// Copyright Sam Bonifacio. All Rights Reserved.

using UnrealBuildTool;

public class AutoSettingsCommonUITests : ModuleRules
{
	public AutoSettingsCommonUITests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			// CQTest provides the Unreal automation test fixture macros used by this developer-only module.
			"CQTest",
			// Each Unreal module DLL must link the launcher-engine Clang exports used by CQTest's automation base classes.
			"SBClangAutoSettings",
			// AutoSettingsCommonUI is the optional integration module covered by this test module.
			"AutoSettingsCommonUI",
			// CommonUI is the optional integration dependency isolated to this test module.
			"CommonUI",
			// Slate exposes the application-level focus delegate used by the combo box to detect popup close.
			"Slate",
			// SlateCore supplies shared widget and focus-path types used by Slate focus delegates.
			"SlateCore",
			// UMG provides the widget lifecycle APIs used to model removal from and re-addition to the UI.
			"UMG"
		});
	}
}
