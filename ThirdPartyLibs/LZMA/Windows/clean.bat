rmdir /s /q .vs

del *.sdf
del *.VC.db

rmdir /s /q Debug

del Release\*.obj
del Release\*.log
rmdir /s /q Release\lzma.tlog

del x64\Release\*.obj
del x64\Release\*.log
rmdir /s /q x64\Release\lzma.tlog

del ARM\Release\*.obj
del ARM\Release\*.log
rmdir /s /q ARM\Release\lzma.tlog

del Emscripten\Release\*.o
del Emscripten\Release\*.log
del Emscripten\Release\*.tlog
del Emscripten\Release\*.lastbuildstate