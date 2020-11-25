@echo off
call "c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cd %~dp0
msbuild "blend2d-static\blend2d.sln" /p:Configuration=Release /p:Platform="x64"
cd %~dp0
