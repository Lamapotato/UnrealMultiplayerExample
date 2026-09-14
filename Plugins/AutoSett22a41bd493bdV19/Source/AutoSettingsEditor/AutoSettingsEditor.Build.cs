// Copyright Sam Bonifacio. All Rights Reserved.

using UnrealBuildTool;

public class AutoSettingsEditor : ModuleRules
{
	public AutoSettingsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		if (Target.Type != TargetType.Editor)
		{
			throw new BuildException("Unable to instantiate AutoSettingsEditor module for non-editor targets.");
		}

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				// ... add public include paths required here ...
			}
		);


		PrivateIncludePaths.AddRange(
			new string[]
			{
				// ... add other private include paths required here ...
			}
		);


		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"EditorSubsystem"
				// ... add other public dependencies that you statically link with here ...
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"AssetTools",
				"CoreUObject",
				"Engine",
				"PropertyEditor",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"ToolWidgets",
				"AutoSettings",
				"AutoSettingsCore",
				"AutoSettingsConsole",
				"AutoSettingsInput",
				"AutoSettingsUI",
				// Each Unreal module DLL must link the launcher-engine Clang exports it imports from Core headers.
				"SBClangAutoSettings",
				"SharedSettingsWidgets"
				// ... add private dependencies that you statically link with here ...
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}
