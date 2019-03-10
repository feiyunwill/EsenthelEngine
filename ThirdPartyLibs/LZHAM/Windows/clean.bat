rmdir /s /q .vs

del *.sdf
del *.VC.db

del Release\*.obj
del Release\*.log
del Release\*.tlog
del Release\*.lastbuildstate
rmdir /s /q Release\lzham.tlog

del x64\Release\*.obj
del x64\Release\*.log
rmdir /s /q x64\Release\lzham.tlog

del "Release Universal\*.obj"
del "Release Universal\*.log"
del "Release Universal\*.tlog"
del "Release Universal\*.lastbuildstate"
rmdir /s /q "Release Universal\lzham.tlog"

del "x64\Release Universal\*.obj"
del "x64\Release Universal\*.log"
rmdir /s /q "x64\Release Universal\lzham.tlog"

del "ARM\Release Universal\*.obj"
del "ARM\Release Universal\*.log"
rmdir /s /q "ARM\Release Universal\lzham.tlog"

del Emscripten\Release\*.o
del Emscripten\Release\*.tlog
del Emscripten\Release\*.lastbuildstate