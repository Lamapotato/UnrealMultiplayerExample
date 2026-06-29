// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UME : ModuleRules
{
	public UME(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"OnlineSubsystem",
			"AdvancedSessions",
			"AdvancedSteamSessions",
            "InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");

        PublicIncludePaths.AddRange(new string[] {
			"UME",
			"UME/Variant_Platforming",
			"UME/Variant_Platforming/Animation",
			"UME/Variant_Combat",
			"UME/Variant_Combat/AI",
			"UME/Variant_Combat/Animation",
			"UME/Variant_Combat/Gameplay",
			"UME/Variant_Combat/Interfaces",
			"UME/Variant_Combat/UI",
			"UME/Variant_SideScrolling",
			"UME/Variant_SideScrolling/AI",
			"UME/Variant_SideScrolling/Gameplay",
			"UME/Variant_SideScrolling/Interfaces",
			"UME/Variant_SideScrolling/UI"
		});

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
