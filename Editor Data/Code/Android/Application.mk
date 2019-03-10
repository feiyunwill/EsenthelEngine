APP_PLATFORM := android-18
APP_ABI      := ABI
APP_STL      := c++_static

RELEASE_CONDITION # RELEASE
	APP_OPTIM   :=   release # if not specified then default value is taken from "AndroidManifest.xml" file 'Debuggable' value
	APP_CFLAGS  := -DDEBUG=0 -O3
	APP_CPPFLAGS:= -DDEBUG=0 -O3
else              # DEBUG
	APP_OPTIM   :=   debug   # if not specified then default value is taken from "AndroidManifest.xml" file 'Debuggable' value
	APP_CFLAGS  := -DDEBUG=1
	APP_CPPFLAGS:= -DDEBUG=1
endif
