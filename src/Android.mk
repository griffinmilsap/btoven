LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE   := arm
LOCAL_MODULE     := libbtoven
LOCAL_CFLAGS 	 := $(EXTERNAL_CFLAGS) -std=c99
LOCAL_C_INCLUDES := $(LOCAL_PATH)/kissfft  
LOCAL_SRC_FILES  := \
	audiobuffer.c \
 	audioformat.c \
 	biquad.c \
 	btoven.c \
 	comb.c \
	error.c \
	processing.c \
	processing_software.c \
	window.c \
	kissfft/kiss_fft.c \
	kissfft/kiss_fftr.c

include $(BUILD_STATIC_LIBRARY)
