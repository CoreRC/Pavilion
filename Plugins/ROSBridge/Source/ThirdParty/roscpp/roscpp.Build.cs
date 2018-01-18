// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class roscpp : ModuleRules
{
    public roscpp(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;
        bUseRTTI = true;
        
        if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "lib", "libroscpp.dylib"));
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "lib", "libroscpp_serialization.dylib"));
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "lib", "libroslib.dylib"));
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "lib", "librospack.dylib"));
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "lib", "librosconsole.dylib"));
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "lib", "librostime.dylib"));
        }
    }
}
