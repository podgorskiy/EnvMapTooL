#!lua

   project "targa"
      kind "StaticLib"
      language "C++"
      files { "*.h", "*.cpp" }

      targetdir("lib")

      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }


