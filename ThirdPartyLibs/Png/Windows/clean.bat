rmdir /s /q .vs
rmdir /s /q Debug
rmdir /s /q "ARM\DLL Debug"

del *.sdf
del *.VC.db

del "LIB Release\*.obj"
del "LIB Release\*.log"
del "LIB Release\*.pch"
rmdir /s /q "LIB Release\png.tlog"

del "x64\LIB Release\*.obj"
del "x64\LIB Release\*.log"
del "x64\LIB Release\*.pch"
rmdir /s /q "x64\LIB Release\png.tlog"

del "ARM\LIB Release\*.obj"
del "ARM\LIB Release\*.log"
del "ARM\LIB Release\*.pch"
rmdir /s /q "ARM\LIB Release\png.tlog"

del "Emscripten\LIB Release\*.o"
del "Emscripten\LIB Release\*.tlog"
del "Emscripten\LIB Release\*.lastbuildstate"
rmdir /s /q ipch