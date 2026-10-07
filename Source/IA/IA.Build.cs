// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class IA : ModuleRules
{
	public IA(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"IA",
			"IA/Variant_Platforming",
			"IA/Variant_Platforming/Animation",
			"IA/Variant_Combat",
			"IA/Variant_Combat/AI",
			"IA/Variant_Combat/Animation",
			"IA/Variant_Combat/Gameplay",
			"IA/Variant_Combat/Interfaces",
			"IA/Variant_Combat/UI",
			"IA/Variant_SideScrolling",
			"IA/Variant_SideScrolling/AI",
			"IA/Variant_SideScrolling/Gameplay",
			"IA/Variant_SideScrolling/Interfaces",
			"IA/Variant_SideScrolling/UI",
			"IA/NPC"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
