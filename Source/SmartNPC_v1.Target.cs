// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SmartNPC_v1Target : TargetRules
{
	public SmartNPC_v1Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
        //DefaultBuildSettings = BuildSettingsVersion.V2;
        //ExtraModuleNames.Add("SmartNPC_v1");


        //Type = TargetType.Editor;

        // ����ΪUE5.5�Ƽ����ã���������ͻ
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        CppStandard = CppStandardVersion.Cpp20;
        bOverrideBuildEnvironment = true;

        ExtraModuleNames.Add("SmartNPC_v1");
    }
}
