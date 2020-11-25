::
:: This script compiles Rebol3-Blend2D extension (release x64)
::

@echo off
call "c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cd %~dp0
msbuild "ext-b2d-x64-st.sln" /p:Configuration=Release /p:Platform="x64"
cd %~dp0
