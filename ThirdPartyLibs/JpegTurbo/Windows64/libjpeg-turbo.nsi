!include x64.nsh
Name "libjpeg-turbo SDK for Visual C++ 64-bit"
OutFile "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\${BUILDDIR}libjpeg-turbo-1.5.1-vc64.exe"
InstallDir c:\libjpeg-turbo64

SetCompressor bzip2

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "libjpeg-turbo SDK for Visual C++ 64-bit (required)"
!ifdef WIN64
	${If} ${RunningX64}
	${DisableX64FSRedirection}
	${Endif}
!endif
	SectionIn RO
!ifdef GCC
	IfFileExists $SYSDIR/libturbojpeg.dll exists 0
!else
	IfFileExists $SYSDIR/turbojpeg.dll exists 0
!endif
	goto notexists
	exists:
!ifdef GCC
	MessageBox MB_OK "An existing version of the libjpeg-turbo SDK for Visual C++ 64-bit is already installed.  Please uninstall it first."
!else
	MessageBox MB_OK "An existing version of the libjpeg-turbo SDK for Visual C++ 64-bit or the TurboJPEG SDK is already installed.  Please uninstall it first."
!endif
	quit

	notexists:
	SetOutPath $SYSDIR
!ifdef GCC
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\libturbojpeg.dll"
!else
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\${BUILDDIR}turbojpeg.dll"
!endif
	SetOutPath $INSTDIR\bin
!ifdef GCC
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\libturbojpeg.dll"
!else
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\${BUILDDIR}turbojpeg.dll"
!endif
!ifdef GCC
	File "/oname=libjpeg-8.dll" "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\sharedlib\libjpeg-*.dll"
!else
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\sharedlib\${BUILDDIR}jpeg8.dll"
!endif
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\sharedlib\${BUILDDIR}cjpeg.exe"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\sharedlib\${BUILDDIR}djpeg.exe"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\sharedlib\${BUILDDIR}jpegtran.exe"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\${BUILDDIR}tjbench.exe"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\${BUILDDIR}rdjpgcom.exe"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\${BUILDDIR}wrjpgcom.exe"
	SetOutPath $INSTDIR\lib
!ifdef GCC
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\libturbojpeg.dll.a"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\libturbojpeg.a"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\sharedlib\libjpeg.dll.a"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\libjpeg.a"
!else
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\${BUILDDIR}turbojpeg.lib"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\${BUILDDIR}turbojpeg-static.lib"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\sharedlib\${BUILDDIR}jpeg.lib"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\${BUILDDIR}jpeg-static.lib"
!endif
!ifdef JAVA
	SetOutPath $INSTDIR\classes
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\java\${BUILDDIR}turbojpeg.jar"
!endif
	SetOutPath $INSTDIR\include
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/Windows64\jconfig.h"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/lib\jerror.h"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/lib\jmorecfg.h"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/lib\jpeglib.h"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/lib\turbojpeg.h"
	SetOutPath $INSTDIR\doc
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/lib\README.ijg"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/lib\README.md"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/lib\LICENSE.md"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/lib\example.c"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/lib\libjpeg.txt"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/lib\structure.txt"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/lib\usage.txt"
	File "C:/Esenthel/ThirdPartyLibs/JpegTurbo/lib\wizard.txt"

	WriteRegStr HKLM "SOFTWARE\64 1.5.1" "Install_Dir" "$INSTDIR"

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\64 1.5.1" "DisplayName" "libjpeg-turbo SDK v1.5.1 for Visual C++ 64-bit"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\64 1.5.1" "UninstallString" '"$INSTDIR\uninstall_1.5.1.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\64 1.5.1" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\64 1.5.1" "NoRepair" 1
	WriteUninstaller "uninstall_1.5.1.exe"
SectionEnd

Section "Uninstall"
!ifdef WIN64
	${If} ${RunningX64}
	${DisableX64FSRedirection}
	${Endif}
!endif

	SetShellVarContext all

	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\64 1.5.1"
	DeleteRegKey HKLM "SOFTWARE\64 1.5.1"

!ifdef GCC
	Delete $INSTDIR\bin\libjpeg-8.dll
	Delete $INSTDIR\bin\libturbojpeg.dll
	Delete $SYSDIR\libturbojpeg.dll
	Delete $INSTDIR\lib\libturbojpeg.dll.a"
	Delete $INSTDIR\lib\libturbojpeg.a"
	Delete $INSTDIR\lib\libjpeg.dll.a"
	Delete $INSTDIR\lib\libjpeg.a"
!else
	Delete $INSTDIR\bin\jpeg8.dll
	Delete $INSTDIR\bin\turbojpeg.dll
	Delete $SYSDIR\turbojpeg.dll
	Delete $INSTDIR\lib\jpeg.lib
	Delete $INSTDIR\lib\jpeg-static.lib
	Delete $INSTDIR\lib\turbojpeg.lib
	Delete $INSTDIR\lib\turbojpeg-static.lib
!endif
!ifdef JAVA
	Delete $INSTDIR\classes\turbojpeg.jar
!endif
	Delete $INSTDIR\bin\cjpeg.exe
	Delete $INSTDIR\bin\djpeg.exe
	Delete $INSTDIR\bin\jpegtran.exe
	Delete $INSTDIR\bin\tjbench.exe
	Delete $INSTDIR\bin\rdjpgcom.exe
	Delete $INSTDIR\bin\wrjpgcom.exe
	Delete $INSTDIR\include\jconfig.h"
	Delete $INSTDIR\include\jerror.h"
	Delete $INSTDIR\include\jmorecfg.h"
	Delete $INSTDIR\include\jpeglib.h"
	Delete $INSTDIR\include\turbojpeg.h"
	Delete $INSTDIR\uninstall_1.5.1.exe
	Delete $INSTDIR\doc\README.ijg
	Delete $INSTDIR\doc\README.md
	Delete $INSTDIR\doc\LICENSE.md
	Delete $INSTDIR\doc\example.c
	Delete $INSTDIR\doc\libjpeg.txt
	Delete $INSTDIR\doc\structure.txt
	Delete $INSTDIR\doc\usage.txt
	Delete $INSTDIR\doc\wizard.txt

	RMDir "$INSTDIR\include"
	RMDir "$INSTDIR\lib"
	RMDir "$INSTDIR\doc"
!ifdef JAVA
	RMDir "$INSTDIR\classes"
!endif
	RMDir "$INSTDIR\bin"
	RMDir "$INSTDIR"

SectionEnd
