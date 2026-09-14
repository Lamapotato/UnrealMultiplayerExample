// Copyright Sam Bonifacio. All Rights Reserved.

using UnrealBuildTool;

public class AutoSettingsInput : ModuleRules
{
	public AutoSettingsInput(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
	        "Core", 
	        "CoreUObject", 
	        "Engine", 
	        "InputCore", 
	        "UMG", 
	        "Slate", 
	        "SlateCore", 
	        "GameplayTags", 
	        "DeveloperSettings", 
	        "EnhancedInput",
	        "AutoSettingsCore", 
	        "AutoSettings",
	        "AutoSettingsUI",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
	        "ApplicationCore",
	        // Each Unreal module DLL must link the launcher-engine Clang exports it imports from Core headers.
	        "SBClangAutoSettings",
        });

	}
}
