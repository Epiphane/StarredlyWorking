// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class DeterministicPhysics : ModuleRules
{
    public DeterministicPhysics(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "UMG",
        });

        AddBullet();
    }

    /// Helper to give us ProjectRoot/ThirdParty
    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/")); }
    }

    protected void AddBullet()
    {
        // This is real basic, only for a single platform & config (Win64)
        // If you build for more variants you'll have to do some more work here

        bool bDebug = Target.Configuration == UnrealTargetConfiguration.Debug || Target.Configuration == UnrealTargetConfiguration.DebugGame;
        bool bDevelopment = Target.Configuration == UnrealTargetConfiguration.Development;

        string BuildFolder = bDebug ? "Debug" :
            bDevelopment ? "RelWithDebInfo" : "Release";
        string BuildSuffix = bDebug ? "_Debug" :
            bDevelopment ? "_RelWithDebugInfo" : "";

        // Library path
        string LibrariesPath = Path.Combine(ThirdPartyPath, "lib", "bullet", BuildFolder);
        PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "BulletCollision" + BuildSuffix + ".lib"));
        PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "BulletDynamics" + BuildSuffix + ".lib"));
        PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "LinearMath" + BuildSuffix + ".lib"));

        // Include path (I'm just using the source here since Bullet has mixed src & headers)
        PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "bullet3", "src"));
        PublicDefinitions.Add("WITH_BULLET_BINDING=1");

    }
}
