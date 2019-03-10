rmdir /s /q .vs

del *.sdf
del *.VC.db

del objs\Release\*.obj
del objs\Release\*.log
del objs\Release\*.tlog
del objs\Release\*.lastbuildstate
rmdir /s /q objs\Release\flac.tlog
rmdir /s /q Release

del objs\x64\Release\*.obj
del objs\x64\Release\*.log
rmdir /s /q objs\x64\Release\flac.tlog
rmdir /s /q x64

del "ARM\Release\*.obj"
del "ARM\Release\*.log"
rmdir /s /q "ARM\Release\flac.tlog"

del "Emscripten\Release\*.o"
del "Emscripten\Release\*.log"
del "Emscripten\Release\*.tlog"
del "Emscripten\Release\*.lastbuildstate"

del "x64\Release\*.obj"
del "x64\Release\*.log"
rmdir /s /q "x64\Release\flac.tlog"
