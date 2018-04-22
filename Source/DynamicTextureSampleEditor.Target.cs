// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class DynamicTextureSampleEditorTarget : TargetRules
{
	public DynamicTextureSampleEditorTarget(TargetInfo Target) : base(Target)
    {
        this.Type = TargetType.Editor;

        this.ExtraModuleNames.AddRange(
            new string[] {
                "DynamicTextureSample"
            });

        this.bEnforceIWYU = true;
        this.bCompileLeanAndMeanUE = true;
        this.bForceEnableRTTI = true;
    }

	//
	// TargetRules interface.
	//

}
