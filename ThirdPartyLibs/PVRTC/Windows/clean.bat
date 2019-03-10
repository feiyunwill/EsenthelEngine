del *.sdf
rmdir /s /q .vs
rmdir /s /q Debug

del Release\*.obj
del Release\*.log
del Release\*.tlog
del Release\*.lastbuildstate
rmdir /s /q Release\pvrtc.tlog

del x64\Release\*.obj
del x64\Release\*.log
del x64\Release\*.tlog
del x64\Release\*.lastbuildstate
rmdir /s /q x64\Release\pvrtc.tlog