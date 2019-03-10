LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_ARM_NEON := true # force NEON usage for all files
include libvpx/build/make/Android.mk
