#!lua

solution "EnvMapTool"
   configurations { "Debug", "Release" }
   location ("projects/" .. _ACTION)

   include "libs/libpng"
   include "libs/zlib"
   include "libs/libjpeg"
   
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


