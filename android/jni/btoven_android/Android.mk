LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE   := arm
LOCAL_MODULE     := btoven_android
LOCAL_CFLAGS     := $(EXTERNAL_CFLAGS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../inc
LOCAL_SRC_FILES  := btoven_interface.c
LOCAL_LDLIBS     += -lOpenSLES
LOCAL_LDLIBS     += -llog
LOCAL_LDLIBS     += -landroid

LOCAL_SHARED_LIBRARIES := btoven

include $(BUILD_SHARED_LIBRARY)
