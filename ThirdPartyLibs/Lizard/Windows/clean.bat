rmdir /s /q .vs

del *.sdf
del *.VC.db

del Release\*.obj
del Release\*.log
del Release\*.tlog
del Release\*.lastbuildstate
rmdir /s /q Release\Lizard.tlog

del x64\Release\*.obj
del x64\Release\*.log
rmdir /s /q x64\Release\Lizard.tlog

del "ARM\Release\*.obj"
del "ARM\Release\*.log"
rmdir /s /q "ARM\Release\Lizard.tlog"

del Emscripten\Release\*.o
del Emscripten\Release\*.tlog
del Emscripten\Release\*.lastbuildstate