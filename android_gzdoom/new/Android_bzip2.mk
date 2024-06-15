
LOCAL_PATH := $(call my-dir)/bzip2


include $(CLEAR_VARS)


LOCAL_MODULE    := bzip2_dev

LOCAL_CFLAGS :=   -O2  

LOCAL_CFLAGS += -march=armv7-a -mfloat-abi=softfp  -DBZ_NO_STDIO 

LOCAL_C_INCLUDES :=   $(LOCAL_PATH)/include/



LOCAL_SRC_FILES =  	\
	  blocksort.c \
    bzlib.c \
    compress.c \
    crctable.c \
    decompress.c \
    huffman.c \
    randtable.c \

include $(BUILD_STATIC_LIBRARY)








