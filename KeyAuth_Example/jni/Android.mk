LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := keyauth_static
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libkeyauth.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := keyauth_test
LOCAL_SRC_FILES := main.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_STATIC_LIBRARIES := keyauth_static
LOCAL_LDLIBS := -llog -landroid

include $(BUILD_EXECUTABLE)
