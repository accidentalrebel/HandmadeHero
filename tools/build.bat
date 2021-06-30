@echo off
mkdir w:\build
pushd w:\build
call shell.bat
cl -DboHANDMADE_WIN32=1 /Zi w:\code\win32_handmade.cpp user32.lib gdi32.lib
popd
