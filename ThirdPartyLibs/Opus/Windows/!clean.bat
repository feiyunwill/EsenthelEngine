rmdir /s /q .vs
rmdir /s /q Debug
rmdir /s /q "Generated Files"

del *.sdf
del *.VC.db
del ARM\Release\*.pri

rmdir /s /q ARM\Release\celt
rmdir /s /q ARM\Release\opus
rmdir /s /q ARM\Release\silk_common
rmdir /s /q ARM\Release\silk_float
rmdir /s /q ARM\Release\silk_fixed

rmdir /s /q Emscripten\Release\celt
rmdir /s /q Emscripten\Release\opus
rmdir /s /q Emscripten\Release\silk_common
rmdir /s /q Emscripten\Release\silk_float
rmdir /s /q Emscripten\Release\silk_fixed

rmdir /s /q Win32\Debug
rmdir /s /q Win32\Release\celt
rmdir /s /q Win32\Release\opus
rmdir /s /q Win32\Release\silk_common
rmdir /s /q Win32\Release\silk_float
rmdir /s /q Win32\Release\silk_fixed

rmdir /s /q x64\Release\celt
rmdir /s /q x64\Release\opus
rmdir /s /q x64\Release\silk_common
rmdir /s /q x64\Release\silk_float
rmdir /s /q x64\Release\silk_fixed
