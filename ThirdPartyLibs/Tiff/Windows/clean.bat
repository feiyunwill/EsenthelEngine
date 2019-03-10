rmdir /s /q .vs

del *.sdf
del *.VC.db

rmdir /s /q Debug

del Release\*.obj
del Release\*.log
rmdir /s /q Release\tiff.tlog

del x64\Release\*.obj
del x64\Release\*.log
rmdir /s /q x64\Release\tiff.tlog

del ARM\Release\*.obj
del ARM\Release\*.log
rmdir /s /q ARM\Release\tiff.tlog

del Emscripten\Release\*.o
del Emscripten\Release\*.log
del Emscripten\Release\*.tlog
del Emscripten\Release\*.lastbuildstate