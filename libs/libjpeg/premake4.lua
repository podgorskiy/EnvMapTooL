#!lua

   project "jpeg"
      kind "StaticLib"
      language "C"
      files { "*.h", "*.c" }
      excludes { 
         "jmem*",
         "cjpeg*" 
      }
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


