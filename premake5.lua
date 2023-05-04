-- premake5.lua
workspace "RayTracingFun"
   architecture "x64"
   configurations { "RayTracingFun", "Release", "Dist" }
   startproject "WalnutApp"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "RayTracingFun"