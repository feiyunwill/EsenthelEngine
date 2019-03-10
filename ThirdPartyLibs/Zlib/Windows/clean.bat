rmdir /s /q .vs

del *.sdf
del *.VC.db

del Release\*.obj
del Release\*.log
rmdir /s /q Release\zlib.tlog

del x64\Release\*.obj
del x64\Release\*.log
rmdir /s /q x64\Release\zlib.tlog

del ARM\ReleaseWithoutAsm\*.obj
del ARM\ReleaseWithoutAsm\*.log
del ARM\ReleaseWithoutAsm\*.pdb
rmdir /s /q ARM\ReleaseWithoutAsm\zlib.tlog

del Emscripten\Release\*.o
del Emscripten\Release\*.log
del Emscripten\Release\*.tlog
del Emscripten\Release\*.lastbuildstate