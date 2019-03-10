rmdir /s /q .vs

del *.sdf
del *.VC.db

del "Release Multithreaded\*.obj"
del "Release Multithreaded\*.log"
del "Release Multithreaded\*.tlog"
del "Release Multithreaded\*.lastbuildstate"
rmdir /s /q "Release Multithreaded\freetype.tlog"

del "x64\Release Multithreaded\*.obj"
del "x64\Release Multithreaded\*.log"
del "x64\Release Multithreaded\*.tlog"
del "x64\Release Multithreaded\*.lastbuildstate"
rmdir /s /q "x64\Release Multithreaded\freetype.tlog"
