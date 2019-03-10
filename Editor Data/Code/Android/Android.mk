LOCAL_PATH := $(call my-dir)

# EsenthelEngine
include $(CLEAR_VARS)
LOCAL_MODULE := EsenthelEngine 
LOCAL_SRC_FILES := ESENTHEL_LIB_PATH/EsenthelEngine-$(TARGET_ARCH_ABI).a
include $(PREBUILT_STATIC_LIBRARY)

EXTERNAL_LIBS

# Project
include $(CLEAR_VARS)
LOCAL_MODULE           := Project
LOCAL_SRC_FILES        := Main.cpp
LOCAL_LDLIBS           := -llog -landroid -lEGL -lGLESv3 -lz -lOpenSLES -Wl,--no-wchar-size-warning
LOCAL_STATIC_LIBRARIES := EsenthelEngine android_native_app_glue cpufeatures EXTERNAL_STATIC_LIB_NAMES
LOCAL_SHARED_LIBRARIES := EXTERNAL_SHARED_LIB_NAMES
LOCAL_CFLAGS           := -I.. INCLUDE_DIRS -fshort-wchar -ffast-math -fomit-frame-pointer -fpermissive
LOCAL_CPPFLAGS         := -I.. INCLUDE_DIRS -fshort-wchar -ffast-math -fomit-frame-pointer -fpermissive -ffriend-injection -std=c++11 -Wno-invalid-offsetof -Wno-comment -Wno-parentheses -Wno-switch -Wno-address-of-temporary -Wno-null-dereference -Wno-int-to-void-pointer-cast
LOCAL_CPP_FEATURES     := rtti
LOCAL_ARM_NEON         := true # force NEON usage for all files
LOCAL_DISABLE_FATAL_LINKER_WARNINGS := true

include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)
$(call import-module, android/cpufeatures)
