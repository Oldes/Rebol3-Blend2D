@echo off
::
:: Initialize Visual Studio environment
::
call "c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cd %~dp0
::
:: Prepare VS probject for Blend2D static library using it's cmake
::
mkdir msvc\blend2d-static
cd msvc\blend2d-static
cmake ..\..\blend2d\ -DCMAKE_BUILD_TYPE=Release -DBLEND2D_STATIC=TRUE
cd %~dp0
::
:: Build Blend2D static library
::
msbuild "msvc\blend2d-static\blend2d.sln" /p:Configuration=Release /p:Platform="x64"
cd %~dp0 
::
:: Build Rebol3-Blend2D extension (release x64)
::
msbuild "msvc\ext-b2d-x64-st.sln" /p:Configuration=Release /p:Platform="x64"
cd %~dp0
::
:: Copy result to project root
::
copy msvc\Release-x64\ext-b2d-x64-st.dll r3-blend2d-x64.dll