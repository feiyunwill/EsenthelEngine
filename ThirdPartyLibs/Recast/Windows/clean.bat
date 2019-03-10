rmdir /s /q .vs

del *.sdf
del *.VC.db

del Release\*.obj
del Release\*.log
rmdir /s /q Release\recast.tlog

del x64\Release\*.obj
del x64\Release\*.log
rmdir /s /q x64\Release\recast.tlog

del "Release Universal\*.obj"
del "Release Universal\*.log"
rmdir /s /q "Release Universal\recast.tlog"

del "x64\Release Universal\*.obj"
del "x64\Release Universal\*.log"
rmdir /s /q "x64\Release Universal\recast.tlog"

del "ARM\Release Universal\*.obj"
del "ARM\Release Universal\*.log"
rmdir /s /q "ARM\Release Universal\recast.tlog"

del Emscripten\Release\*.o
del Emscripten\Release\*.tlog
del Emscripten\Release\*.lastbuildstate