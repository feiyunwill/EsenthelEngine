copy vpx_config.h libvpx\vpx_config.h

del Windows32\makefile
del Windows32\.*
del Windows32\*.log
del Windows32\*.mk
del Windows32\*.def
del Windows32\*.sdf
del Windows32\*.sln
del Windows32\*.vcproj
del Windows32\*.vcxproj
del Windows32\*.filters
del Windows32\*.asm
del Windows32\*.c
del Windows32\*.h
del Windows32\*.htm
del Windows32\Win32\Release\tiny_ssim.*
rmdir /s /q Windows32\.vs
rmdir /s /q Windows32\Backup
rmdir /s /q Windows32\Win32\Release\vpx
rmdir /s /q Windows32\Win32\Release\tiny_ssim

del Windows64\makefile
del Windows64\.*
del Windows64\*.log
del Windows64\*.mk
del Windows64\*.def
del Windows64\*.sdf
del Windows64\*.sln
del Windows64\*.vcproj
del Windows64\*.vcxproj
del Windows64\*.filters
del Windows64\*.asm
del Windows64\*.c
del Windows64\*.h
del Windows64\*.htm
del Windows64\x64\Release\tiny_ssim.*
rmdir /s /q Windows64\.vs
rmdir /s /q Windows64\Backup
rmdir /s /q Windows64\x64\Release\vpx
rmdir /s /q Windows64\x64\Release\tiny_ssim

del WindowsARM\makefile
del WindowsARM\.*
del WindowsARM\*.log
del WindowsARM\*.mk
del WindowsARM\*.def
del WindowsARM\*.sdf
del WindowsARM\*.sln
del WindowsARM\*.vcproj
del WindowsARM\*.vcxproj
del WindowsARM\*.filters
del WindowsARM\*.asm
del WindowsARM\*.c
del WindowsARM\*.h
del WindowsARM\*.htm
del WindowsARM\ARM\Release\*.pri
del WindowsARM\ARM\Release\tiny_ssim.*
rmdir /s /q WindowsARM\.vs
rmdir /s /q WindowsARM\Backup
rmdir /s /q WindowsARM\vpx_dsp
rmdir /s /q "WindowsARM\Generated Files"
rmdir /s /q WindowsARM\ARM\Debug
rmdir /s /q WindowsARM\ARM\Release\vpx
rmdir /s /q WindowsARM\ARM\Release\tiny_ssim
