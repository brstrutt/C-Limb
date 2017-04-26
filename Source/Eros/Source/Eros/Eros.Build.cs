// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Eros : ModuleRules
{
	public Eros(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "AIModule", "UMG", "LevelSequence", "MovieScene" });

        PrivateDependencyModuleNames.AddRange(new string[] { "CableComponent" });
		PrivateIncludePathModuleNames.AddRange(new string[] { "CableComponent" });
	}
}
