del *.sdf
rmdir /s /q .vs

del Bin\Desktop_2015\Win32\Release\*.obj
del Bin\Desktop_2015\Win32\Release\*.pch
del Bin\Desktop_2015\Win32\Release\*.pdb
del Bin\Desktop_2015\Win32\Release\*.log
rmdir /s /q Bin\Desktop_2015\Win32\Release\Effects11.tlog

del Bin\Desktop_2015\x64\Release\*.obj
del Bin\Desktop_2015\x64\Release\*.pch
del Bin\Desktop_2015\x64\Release\*.pdb
del Bin\Desktop_2015\x64\Release\*.log
rmdir /s /q Bin\Desktop_2015\x64\Release\Effects11.tlog

del Bin\Windows10\Win32\Release\*.obj
del Bin\Windows10\Win32\Release\*.pch
del Bin\Windows10\Win32\Release\*.pdb
del Bin\Windows10\Win32\Release\*.log
del Bin\Windows10\Win32\Release\*.pri
del Bin\Windows10\Win32\Release\*.resfiles
del Bin\Windows10\Win32\Release\*.intermediate
del Bin\Windows10\Win32\Release\*.txt
del Bin\Windows10\Win32\Release\*.xml
del Bin\Windows10\Win32\Release\*.state
rmdir /s /q Bin\Windows10\Win32\Release\Effects11.tlog

del Bin\Windows10\x64\Release\*.obj
del Bin\Windows10\x64\Release\*.pch
del Bin\Windows10\x64\Release\*.pdb
del Bin\Windows10\x64\Release\*.log
del Bin\Windows10\x64\Release\*.pri
del Bin\Windows10\x64\Release\*.resfiles
del Bin\Windows10\x64\Release\*.intermediate
del Bin\Windows10\x64\Release\*.txt
del Bin\Windows10\x64\Release\*.xml
del Bin\Windows10\x64\Release\*.state
rmdir /s /q Bin\Windows10\x64\Release\Effects11.tlog

del Bin\Windows10\ARM\Release\*.obj
del Bin\Windows10\ARM\Release\*.pch
del Bin\Windows10\ARM\Release\*.pdb
del Bin\Windows10\ARM\Release\*.log
del Bin\Windows10\ARM\Release\*.pri
del Bin\Windows10\ARM\Release\*.resfiles
del Bin\Windows10\ARM\Release\*.intermediate
del Bin\Windows10\ARM\Release\*.txt
del Bin\Windows10\ARM\Release\*.xml
del Bin\Windows10\ARM\Release\*.state
rmdir /s /q Bin\Windows10\ARM\Release\Effects11.tlog

rmdir /s /q "Generated Files"
rmdir /s /q "ipch"