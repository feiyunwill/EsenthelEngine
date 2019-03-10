rmdir /s /q .vs

del *.sdf

del Release\*.obj
del Release\*.log
rmdir /s /q Release\theora.tlog

del x64\Release\*.obj
del x64\Release\*.log
rmdir /s /q x64\Release\theora.tlog

del ARM\Release\*.obj
del ARM\Release\*.log
rmdir /s /q ARM\Release\theora.tlog

del Emscripten\Release\*.o
del Emscripten\Release\*.tlog
del Emscripten\Release\*.lastbuildstate