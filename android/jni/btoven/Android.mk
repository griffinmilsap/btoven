LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE   := arm
LOCAL_MODULE     := btoven
LOCAL_CFLAGS 	 := $(EXTERNAL_CFLAGS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../inc  $(LOCAL_PATH)/../../../src/kissfft  
LOCAL_LDLIBS     += -llog
LOCAL_SRC_FILES  := \
	../../../src/audiobuffer.c \
 	../../../src/audioformat.c \
 	../../../src/biquad.c \
 	../../../src/btoven.c \
 	../../../src/comb.c \
	../../../src/error.c \
	../../../src/processing.c \
	../../../src/window.c \
	../../../src/kissfft/kiss_fft.c \
	../../../src/kissfft/kiss_fftr.c

include $(BUILD_SHARED_LIBRARY)
