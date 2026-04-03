LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := keyauth

LOCAL_CFLAGS := -std=c++17
LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_CPPFLAGS := -std=c++17
LOCAL_CPPFLAGS += -fvisibility=hidden

LOCAL_CPPFLAGS += -w -s -Wno-error=format-security -fvisibility=hidden -Werror -std=c++17
LOCAL_CPPFLAGS += -Wno-error=c++11-narrowing -fpermissive -Wall -fexceptions

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src/internal_headers

LOCAL_SRC_FILES := \
    src/KeyAuth.cpp

include $(BUILD_STATIC_LIBRARY)
