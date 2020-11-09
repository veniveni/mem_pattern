LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := test
LOCAL_SRC_FILES := main.c

LOCAL_CFLAGS        += -fPIE -Wall -O0
LOCAL_LDFLAGS       += -fPIE -pie

include $(BUILD_EXECUTABLE)