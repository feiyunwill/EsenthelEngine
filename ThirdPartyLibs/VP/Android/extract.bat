rmdir /s /q obj

mkdir obj\local\armeabi-v7a
cd obj\local\armeabi-v7a
..\..\..\..\..\..\Engine\Android\cygwin\ar.exe -x ..\..\..\armeabi-v7a.a
cd ..\..\..

mkdir obj\local\arm64-v8a
cd obj\local\arm64-v8a
..\..\..\..\..\..\Engine\Android\cygwin\ar.exe -x ..\..\..\arm64-v8a.a
cd ..\..\..

mkdir obj\local\x86
cd obj\local\x86
..\..\..\..\..\..\Engine\Android\cygwin\ar.exe -x ..\..\..\x86.a
cd ..\..\..