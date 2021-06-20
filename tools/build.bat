@echo off
mkdir w:\build
pushd w:\build
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
cl /Zi w:\code\win32_handmade.cpp user32.lib
popd
