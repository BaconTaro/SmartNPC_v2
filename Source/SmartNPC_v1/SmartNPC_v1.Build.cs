// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SmartNPC_v1 : ModuleRules
{
	public SmartNPC_v1(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "HTTP", "Json", "JsonUtilities" });
	}
}
