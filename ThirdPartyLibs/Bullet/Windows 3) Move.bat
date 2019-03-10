mkdir "Windows\ARM\Release Universal"
mkdir "Windows\Release"
mkdir "Windows\Release Universal"
mkdir "Windows\x64\Release"
mkdir "Windows\x64\Release Universal"
mkdir "Windows\Emscripten\Release"

move "lib\build3\vs2010\ARM\Release Universal\BulletCollision\*.lib" "Windows\ARM\Release Universal"
move "lib\build3\vs2010\ARM\Release Universal\BulletDynamics\*.lib" "Windows\ARM\Release Universal"
move "lib\build3\vs2010\ARM\Release Universal\LinearMath\*.lib" "Windows\ARM\Release Universal"

move "lib\build3\vs2010\Release\BulletCollision\*.lib" "Windows\Release"
move "lib\build3\vs2010\Release\BulletDynamics\*.lib" "Windows\Release"
move "lib\build3\vs2010\Release\LinearMath\*.lib" "Windows\Release"

move "lib\build3\vs2010\Release Universal\BulletCollision\*.lib" "Windows\Release Universal"
move "lib\build3\vs2010\Release Universal\BulletDynamics\*.lib" "Windows\Release Universal"
move "lib\build3\vs2010\Release Universal\LinearMath\*.lib" "Windows\Release Universal"

move "lib\build3\vs2010\x64\Release\BulletCollision\*.lib" "Windows\x64\Release"
move "lib\build3\vs2010\x64\Release\BulletDynamics\*.lib" "Windows\x64\Release"
move "lib\build3\vs2010\x64\Release\LinearMath\*.lib" "Windows\x64\Release"

move "lib\build3\vs2010\x64\Release Universal\BulletCollision\*.lib" "Windows\x64\Release Universal"
move "lib\build3\vs2010\x64\Release Universal\BulletDynamics\*.lib" "Windows\x64\Release Universal"
move "lib\build3\vs2010\x64\Release Universal\LinearMath\*.lib" "Windows\x64\Release Universal"

move "lib\build3\vs2010\Emscripten\Release\*.bc" "Windows\Emscripten\Release"