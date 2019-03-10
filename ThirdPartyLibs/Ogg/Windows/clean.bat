rmdir /s /q .vs

del *.sdf

del Release\*.obj
del Release\*.log
del Release\*.tlog
del Release\*.lastbuildstate
rmdir /s /q Release\ogg.tlog

del x64\Release\*.obj
del x64\Release\*.log
rmdir /s /q x64\Release\ogg.tlog

del ARM\Release\*.obj
del ARM\Release\*.log
rmdir /s /q ARM\Release\ogg.tlog

del Emscripten\Release\*.o
del Emscripten\Release\*.tlog
del Emscripten\Release\*.lastbuildstate