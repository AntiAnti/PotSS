// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PotSS : ModuleRules
{
	public PotSS(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AIModule" });

        /******************* __SteamVR_Support__ *******************/
        PublicDependencyModuleNames.AddRange(new string[] { "HeadMountedDisplay", "SteamVR", "SteamVRController" });
        /***********************************************************/
    }
}
