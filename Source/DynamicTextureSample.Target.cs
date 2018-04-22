// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class DynamicTextureSampleTarget : TargetRules
{
	public DynamicTextureSampleTarget(TargetInfo Target) : base(Target)
    {
        this.Type = TargetType.Game;

        this.ExtraModuleNames.AddRange(
            new string[] {
                "DynamicTextureSample"
            });

        this.bEnforceIWYU = true;
        this.bCompileLeanAndMeanUE = true;
        this.bForceEnableRTTI = true;
        this.bForceEnableExceptions = true;
    }

	//
	// TargetRules interface.
	//
}
