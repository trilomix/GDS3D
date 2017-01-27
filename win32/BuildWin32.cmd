@echo off
echo This will only run from within the Visual Studio Command Prompt
msbuild GDS3D.sln
rmdir Debug /Q /S
rmdir Release /Q /S
del GDS3D.pdb