APP_PLATFORM := android-18 # minimum platform 9=Android 2.3 GingerBread GLES2, 18=Android 4.3 JellyBean GLES3
APP_ABI      := armeabi-v7a arm64-v8a # x86 # "armeabi"=32-bit ARMV5 (older devices + emulator 2.3), "armeabi-v7a"=32-bit ARMV7 (newer devices + emulator 4.0), "arm64-v8a"=64-bit ARM, "x86"=32-bit x86
APP_STL      := c++_static
APP_MODULES  := EsenthelEngine # this will force generation of .a lib

ifeq (true, true) # RELEASE
	APP_OPTIM   :=   release # if not specified then default value is taken from "AndroidManifest.xml" file 'Debuggable' value
	APP_CFLAGS  := -DDEBUG=0 -O3
	APP_CPPFLAGS:= -DDEBUG=0 -O3
else              # DEBUG
	APP_OPTIM   :=   debug   # if not specified then default value is taken from "AndroidManifest.xml" file 'Debuggable' value
	APP_CFLAGS  := -DDEBUG=1
	APP_CPPFLAGS:= -DDEBUG=1
endif
