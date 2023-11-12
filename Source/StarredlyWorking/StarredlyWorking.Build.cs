// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StarredlyWorking : ModuleRules
{
	public StarredlyWorking(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
		});
	}
}
