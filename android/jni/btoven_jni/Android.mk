LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE   := arm
LOCAL_MODULE     := btoven_android_jni
LOCAL_CFLAGS     := $(EXTERNAL_CFLAGS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../inc $(LOCAL_PATH)/../btoven_android
LOCAL_SRC_FILES  := jniwrapper.c
LOCAL_LDLIBS     += -llog
LOCAL_LDLIBS     += -landroid

LOCAL_SHARED_LIBRARIES := btoven_android

include $(BUILD_SHARED_LIBRARY)
