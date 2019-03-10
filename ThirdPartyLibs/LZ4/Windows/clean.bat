rmdir /s /q .vs
rmdir /s /q Debug
rmdir /s /q "Generated Files"

del *.sdf
del *.VC.db

del Release\*.obj
del Release\*.log
del Release\*.tlog
del Release\*.lastbuildstate
rmdir /s /q Release\lz4.tlog

del x64\Release\*.obj
del x64\Release\*.log
rmdir /s /q x64\Release\lz4.tlog

del "ARM\Release\*.obj"
del "ARM\Release\*.log"
del "ARM\Release\*.resfiles"
del "ARM\Release\*.txt"
del "ARM\Release\*.xml"
del "ARM\Release\*.intermediate"
rmdir /s /q "ARM\Release\LZ4"
rmdir /s /q "ARM\Release\lz4.tlog"

del Emscripten\Release\*.o
del Emscripten\Release\*.log
del Emscripten\Release\*.tlog
del Emscripten\Release\*.lastbuildstate