rmdir /s /q .vs

del *.sdf
del *.VC.db

del Release\*.obj
del Release\*.log
rmdir /s /q Release\webp.tlog

del x64\Release\*.obj
del x64\Release\*.log
rmdir /s /q x64\Release\webp.tlog

del ARM\Release\*.obj
del ARM\Release\*.log
rmdir /s /q ARM\Release\webp.tlog

del Emscripten\Release\*.o
del Emscripten\Release\*.tlog
del Emscripten\Release\*.log
del Emscripten\Release\*.lastbuildstate