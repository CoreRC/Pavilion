// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at http://mozilla.org/MPL/2.0/.

// Copyright (c) 2018, Fan Jiang <i@fanjiang.me>

using System.IO;
using System.Text.RegularExpressions;
using UnrealBuildTool;

public class ignition : ModuleRules
{
    public ignition(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;
        
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PublicIncludePaths.AddRange(
        new string[] {
            // ... add public include paths required here ...
        }
        );
        
        var base_path = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../ThirdParty/Release/ignition-math"));
        var build_path = "lib";
        var full_build_path = Path.Combine(base_path, build_path);

        if (!Directory.Exists(full_build_path)) {
            Log.TraceError("Invalid build path: " + full_build_path + " (Did you build the 3rdparty module already?)");
        }

        if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDelayLoadDLLs.Add(Path.Combine(full_build_path, "libignition-math4.dylib"));
        }
    }
}
