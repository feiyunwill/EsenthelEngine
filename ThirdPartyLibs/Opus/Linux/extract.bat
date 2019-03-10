rmdir /s /q build
mkdir build\Release
cd build\Release
..\..\..\..\..\Engine\Android\cygwin\ar.exe -x ..\..\libopus.a
..\..\..\..\..\Engine\Android\cygwin\ar.exe -x ..\..\libopusfile.a
cd ..\..