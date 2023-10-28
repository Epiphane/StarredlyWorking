// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StarredlyWorkingEditor : ModuleRules
{
	public StarredlyWorkingEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UnrealEd",
			"SubobjectDataInterface",
			"StarredlyWorking",
			"EditorSubsystem",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Slate",
			"SlateCore"
		});
	}
}
