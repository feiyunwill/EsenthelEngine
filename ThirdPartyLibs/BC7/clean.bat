rmdir /s /q .vs
rmdir /s /q ispc_texcomp\Release
rmdir /s /q ispc_texcomp\x64

del Release\*.obj
del x64\Release\*.obj
del ispc.exe
del ispc_osx