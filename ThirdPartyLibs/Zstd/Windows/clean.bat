rmdir /s /q .vs
rmdir /s /q ARM
rmdir /s /q Debug
rmdir /s /q Emscripten
rmdir /s /q "Generated Files"
rmdir /s /q Release
rmdir /s /q x64

del *.sdf
del *.VC.db

del bin\ARM\Release\zstdlib\*.pri

del Release\*.obj
del Release\*.cod
del Release\*.log
del Release\*.tlog
del Release\*.lastbuildstate
rmdir /s /q Release\zstdlib.tlog

del x64\Release\*.obj
del x64\Release\*.log
del x64\Release\*.cod
rmdir /s /q x64\Release\zstdlib.tlog

del "ARM\Release\*.obj"
del "ARM\Release\*.log"
del "ARM\Release\*.cod"
rmdir /s /q "ARM\Release\zstdlib.tlog"

del Emscripten\Release\*.o
del Emscripten\Release\*.log
del Emscripten\Release\*.tlog
del Emscripten\Release\*.lastbuildstate