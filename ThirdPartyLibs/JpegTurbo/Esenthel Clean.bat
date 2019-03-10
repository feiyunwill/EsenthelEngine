rmdir /s /q Web\.vs
rmdir /s /q Windows32\.vs
rmdir /s /q Windows64\.vs
rmdir /s /q WindowsArm\.vs

del Windows32\*.sdf
del Windows64\*.sdf
del WindowsArm\*.sdf
del Web\*.sdf

del Windows32\*.VC.db
del Windows64\*.VC.db
del WindowsArm\*.VC.db
del Web\*.VC.db

rmdir /s /q Windows32\md5
rmdir /s /q Windows64\md5
rmdir /s /q WindowsArm\md5
rmdir /s /q Web\md5

rmdir /s /q Windows32\CMakeFiles
rmdir /s /q Windows64\CMakeFiles
rmdir /s /q WindowsArm\CMakeFiles
rmdir /s /q Web\CMakeFiles

rmdir /s /q Windows32\SharedLib
rmdir /s /q Windows64\SharedLib
rmdir /s /q WindowsArm\SharedLib
rmdir /s /q Web\SharedLib

rmdir /s /q Windows32\turbojpeg-static.dir
rmdir /s /q Windows64\turbojpeg-static.dir
rmdir /s /q WindowsArm\turbojpeg-static.dir
rmdir /s /q Web\turbojpeg-static.dir

rmdir /s /q Windows32\simd\CMakeFiles
rmdir /s /q Windows64\simd\CMakeFiles
rmdir /s /q WindowsArm\simd\CMakeFiles

rmdir /s /q Windows32\simd\Release
rmdir /s /q Windows64\simd\Release
rmdir /s /q WindowsArm\simd\Release

rmdir /s /q Windows32\simd\Win32
rmdir /s /q Web\Win32

rmdir /s /q Windows64\x64
rmdir /s /q Windows64\simd\x64

del Web\Emscripten\Release\*.tlog
del Web\Emscripten\Release\*.o
del Web\Emscripten\Release\*.lastbuildstate
rmdir /s /q Web\Emscripten\Release\turbojpeg-static.tlog
rmdir /s /q Web\Release
rmdir /s /q Web\simd