import qbs 1.0

DynamicLibrary  {
    id: Audi3DLUTGenerator
    name: "Audi3DLUTGenerator"

    Depends { name: "cpp" }
    //builtByDefault: false


    Properties {
        condition: qbs.targetOS.contains("windows")
        cpp.cxxFlags: base.concat(["-Werror=return-type", "-std=c++11"])
        cpp.libFlags: ["-static-libgcc", "-static-libstdc++"]
        cpp.defines: base.concat(["WIN32"])
        cpp.linkerFlags: base.concat([Audi3DLUTGenerator.sourceDirectory + "/Audi3DLUTGenerator.def", "-s"])
    }

    Properties {
        condition: qbs.targetOS.contains("linux")
        cpp.cxxFlags: base.concat(["-Werror=return-type", "-std=c++11", "-fvisibility=hidden"])
        cpp.libFlags: ["-static-libgcc", "-static-libstdc++"]
        cpp.linkerFlags: base.concat(["-s"])
    }
/*
    Properties {
        condition: qbs.targetOS.contains("osx") && qbs.toolchain.contains("clang")
        cpp.cxxFlags: base.concat(["-std=c++11", "-stdlib=libc++", "-Werror=return-type"]).concat(project.profiling ? ["-pg"] : []).concat(project.sanitize ? ["-fsanitize=address"] : [])
        cpp.linkerFlags: base.concat(["-stdlib=libc++"]).concat(project.profiling ? ["-pg"] : []).concat(project.sanitize ? ["-fsanitize=address"] : [])
        cpp.minimumOsxVersion: "10.7"
    }
  */

    cpp.defines:
    {
        if(qbs.buildVariant.contains("debug"))
            return base.concat(["_DEBUG"]);
        return base;
    }

    files: [
        "*.h",
        "*.cpp",
        "Audi3DLUTGenerator.def",
        "IccProfLib/*.h",
        "IccProfLib/*.cpp",
    ]
}
