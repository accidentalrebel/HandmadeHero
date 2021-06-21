@echo off
mkdir w:\build
pushd w:\build
call shell.bat
cl /Zi w:\code\win32_handmade.cpp user32.lib gdi32.lib
popd
