#!lua

   project "zlib"
      kind "StaticLib"
      language "C"
      files { "*.h", "*.c" }

      targetdir("lib")

      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }


