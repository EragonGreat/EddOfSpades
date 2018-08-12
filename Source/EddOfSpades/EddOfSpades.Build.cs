// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EddOfSpades : ModuleRules
{
	public EddOfSpades(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "ProceduralMeshComponent", "OnlineSubsystem", "OnlineSubsystemUtils", "Sockets", "Networking", "ShaderCore", "RenderCore", "RHI", "RuntimeMeshComponent" });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");
        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
    }
}
