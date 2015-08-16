#!lua

   project "png"
      kind "StaticLib"
      language "C"
      files { "*.h", "*.c" }
      excludes { "example*" }

      targetdir("lib")

      includedirs {
         "../zlib/"
      }

      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }


