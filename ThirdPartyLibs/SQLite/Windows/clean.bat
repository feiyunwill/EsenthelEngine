rmdir /s /q .vs

del *.sdf
del *.VC.db

rmdir /s /q "Generated Files"

del Release\*.obj
del Release\*.log
rmdir /s /q Release\sqlite.tlog

del "Release Universal\*.obj"
del "Release Universal\*.log"
del "Release Universal\*.cache"
del "Release Universal\*.resfiles"
del "Release Universal\*.intermediate"
del "Release Universal\*.xml"
del "Release Universal\*.txt"
del "Release Universal\SQLite\*.pri"
rmdir /s /q "Release Universal\sqlite.tlog"

del x64\Release\*.obj
del x64\Release\*.log
rmdir /s /q x64\Release\sqlite.tlog

del "x64\Release Universal\*.obj"
del "x64\Release Universal\*.log"
del "x64\Release Universal\*.cache"
del "x64\Release Universal\*.resfiles"
del "x64\Release Universal\*.intermediate"
del "x64\Release Universal\*.xml"
del "x64\Release Universal\*.txt"
del "x64\Release Universal\SQLite\*.pri"
rmdir /s /q "x64\Release Universal\sqlite.tlog"

del "ARM\Release Universal\*.obj"
del "ARM\Release Universal\*.log"
del "ARM\Release Universal\*.cache"
del "ARM\Release Universal\*.resfiles"
del "ARM\Release Universal\*.intermediate"
del "ARM\Release Universal\*.xml"
del "ARM\Release Universal\*.txt"
del "ARM\Release Universal\SQLite\*.pri"
rmdir /s /q "ARM\Release Universal\sqlite.tlog"

del Emscripten\Release\*.o
del Emscripten\Release\*.tlog
del Emscripten\Release\*.lastbuildstate