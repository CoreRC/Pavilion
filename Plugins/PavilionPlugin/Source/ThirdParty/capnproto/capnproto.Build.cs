// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at http://mozilla.org/MPL/2.0/.

// Copyright (c) 2018, Fan Jiang <i@fanjiang.me>

using System.IO;
using System.Text.RegularExpressions;
using UnrealBuildTool;

public class capnproto : ModuleRules
{
    public capnproto(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;
        
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PublicIncludePaths.AddRange(
        new string[] {
            // ... add public include paths required here ...
        }
        );
        
        var base_path = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../ThirdParty/Release/capnproto"));
        var build_path = "lib";
        var full_build_path = Path.Combine(base_path, build_path);

        if (!Directory.Exists(full_build_path)) {
            Log.TraceError("Invalid build path: " + full_build_path + " (Did you build the 3rdparty module already?)");
        }

        // Look at all the files in the build path; we need to smartly locate
        // the static library based on the current platform. For dynamic libraries
        // this is more difficult, but for static libraries, it's just .lib or .a
        string [] fileEntries = Directory.GetFiles(full_build_path);
        var pattern = ".*\\.";
        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32)) {
            pattern += "lib";
        }
        else {
            pattern += "a";
        }
        Regex r = new Regex(pattern, RegexOptions.IgnoreCase);
        string full_library_path = null;
        foreach (var file in fileEntries) {
            if (r.Match(file).Success) {
                full_library_path = Path.Combine(full_build_path, file);
                PublicAdditionalLibraries.Add(full_library_path);
            }
        }
        if (full_library_path == null) {
            Log.TraceError("Unable to locate any build libraries in: " + full_build_path);
        }
    }
}
