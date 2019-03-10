rmdir /s /q .vs
rmdir /s /q Debug

del *.sdf
del *.VC.db

del Release\*.obj
del Release\*.log
rmdir /s /q Release\snappy.tlog

del x64\Release\*.obj
del x64\Release\*.log
rmdir /s /q x64\Release\snappy.tlog

del "Release Universal\*.obj"
del "Release Universal\*.log"
rmdir /s /q "Release Universal\snappy.tlog"

del "x64\Release Universal\*.obj"
del "x64\Release Universal\*.log"
rmdir /s /q "x64\Release Universal\snappy.tlog"

del "ARM\Release Universal\*.obj"
del "ARM\Release Universal\*.log"
rmdir /s /q "ARM\Release Universal\snappy.tlog"

del Emscripten\Release\*.o
del Emscripten\Release\*.tlog
del Emscripten\Release\*.lastbuildstate