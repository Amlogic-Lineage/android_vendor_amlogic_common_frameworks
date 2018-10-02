LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:= \
    HdmiCecControl.cpp

LOCAL_C_INCLUDES += \
   $(LOCAL_PATH)/../../systemcontrol/PQ/include \
   $(LOCAL_PATH)/../../systemcontrol \
   $(LOCAL_PATH)/../binder \
   hardware/libhardware/include

LOCAL_SHARED_LIBRARIES := \
    vendor.amlogic.hardware.systemcontrol@1.0 \
    libsystemcontrolservice \
    libcutils \
    libutils \
    liblog \
    libbinder

LOCAL_MODULE:= libhdmi_cec_static

ifeq ($(shell test $(PLATFORM_SDK_VERSION) -ge 26 && echo OK),OK)
LOCAL_PROPRIETARY_MODULE := true
endif
include $(BUILD_STATIC_LIBRARY)
