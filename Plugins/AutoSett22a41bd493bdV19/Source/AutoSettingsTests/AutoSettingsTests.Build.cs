// Copyright Sam Bonifacio. All Rights Reserved.

using UnrealBuildTool;

public class AutoSettingsTests : ModuleRules
{
	public AutoSettingsTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AutoSettingsCore",
			"AutoSettings"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			// CQTest provides the Unreal automation test fixture macros used by this developer-only module.
			"CQTest",
			// AutoSettingsConsole owns the handler lifecycle and CVar utilities exercised by ordering coverage.
			"AutoSettingsConsole",
			// AutoSettingsInput owns the runtime binding trigger covered by the pawn-possessed test fixture.
			"AutoSettingsInput",
			// Each Unreal module DLL must link the launcher-engine Clang exports it imports from Core headers.
			"SBClangAutoSettings"
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new[]
			{
				// AssetRegistry supplies the asset metadata exercised by console-variable asset lifecycle coverage.
				"AssetRegistry",
				// Kismet supplies Blueprint compilation used by console-variable asset lifecycle coverage.
				"Kismet",
				// SBAutomationAutoSettingsEditor owns the shared CQTest future, PIE lifecycle, and editor asset utilities.
				"SBAutomationAutoSettingsEditor",
				// UnrealEd supplies transient-map creation and Blueprint editor APIs used by editor-only coverage.
				"UnrealEd"
			});
		}
	}
}
