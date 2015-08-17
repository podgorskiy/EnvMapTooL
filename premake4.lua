#!lua


function newplatform(plf)
    local name = plf.name
    local description = plf.description
 
    -- Register new platform
    premake.platforms[name] = {
        cfgsuffix = "_"..name,
        iscrosscompiler = true
    }
 
    -- Allow use of new platform in --platfroms
    table.insert(premake.option.list["platform"].allowed, { name, description })
    table.insert(premake.fields.platforms.allowed, name)
 
    -- Add compiler support
    -- gcc
    premake.gcc.platforms[name] = plf.gcc
    --other compilers (?)
end
 

newplatform {
    name = "emscripten",
    description = "emscripten",
    gcc = {
        cc = "emcc",
        cxx = "emcc",
        cppflags = ""
    }
}
 
solution "EnvMapTool"
   configurations { "Debug", "Release" }
   platforms { "native", "emscripten" }

   location ("projects/" .. _ACTION)

   include "libs/libpng"
   include "libs/zlib"
   include "libs/libjpeg"
   --include "libs/targa"
   
   project "EnvMapTool"
      kind "ConsoleApp"
      language "C++"
      files { "src/**.h", "src/**.cpp" }

      targetdir("")

      includedirs {
         "libs/tclap/include/",
         "libs/zlib/",
         "libs/libpng/",
         "libs/libjpeg/"
      }
      
      links { "png", "zlib", "jpeg" }

      configuration "Debug"
         debugdir "."
         debugargs { "-i uffizi_cros.dds" }
         defines { "DEBUG" }
         flags { "Symbols" }

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }


