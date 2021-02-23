######################################################################
# Make shared library IrSdk.so
######################################################################
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

CFLAGS := -Werror




LOCAL_CFLAGS := $(LOCAL_C_INCLUDES:%=-I%)
LOCAL_CFLAGS += -DANDROID_NDK
LOCAL_CFLAGS += -DLOG_NDEBUG
LOCAL_CFLAGS += -DACCESS_RAW_DESCRIPTORS
LOCAL_CFLAGS += -O3 -fstrict-aliasing -fprefetch-loop-arrays
LOCAL_EXPORT_LDLIBS += -llog
LOCAL_ARM_MODE := arm





LOCAL_C_INCLUDES := \
        $(LOCAL_PATH)/ \
        $(LOCAL_PATH)/../ \
        $(LOCAL_PATH)/libusb/ \
        $(LOCAL_PATH)/libusb/libusb \
        $(LOCAL_PATH)/libusb/libusb/os \
        $(LOCAL_PATH)/libusb/../ \
        $(LOCAL_PATH)/libusb/../include \
        $(LOCAL_PATH)/libusb/android


LOCAL_EXPORT_C_INCLUDES := \
    $(LOCAL_PATH)/libusb/ \
    $(LOCAL_PATH)/libusb/libusb


LOCAL_LDLIBS := -llog
LOCAL_LDLIBS += -landroid

#LOCAL_SHARED_LIBRARIES += usb1.0
#LOCAL_SHARED_LIBRARIES += IrUsb
#LOCAL_STATIC_LIBRARIES = libIrUsb_static

LOCAL_SRC_FILES := \
    $(LOCAL_PATH)/libusb/libusb/core.c \
    $(LOCAL_PATH)/libusb/libusb/descriptor.c \
    $(LOCAL_PATH)/libusb/libusb/hotplug.c \
    $(LOCAL_PATH)/libusb/libusb/io.c \
    $(LOCAL_PATH)/libusb/libusb/sync.c \
    $(LOCAL_PATH)/libusb/libusb/strerror.c \
    $(LOCAL_PATH)/libusb/libusb/os/android_usbfs.c \
    $(LOCAL_PATH)/libusb/libusb/os/poll_posix.c \
    $(LOCAL_PATH)/libusb/libusb/os/threads_posix.c \
    $(LOCAL_PATH)/libusb/libusb/os/android_netlink.c \
        $(LOCAL_PATH)/ir_plug_device.c \
        $(LOCAL_PATH)/ir_plug_sdk.c \
        $(LOCAL_PATH)/ir_plug_sdk_algorithm.c \
        $(LOCAL_PATH)/ir_plug_sdk_colormap.c \
        $(LOCAL_PATH)/ir_plug_sdk_colormap_rgb565.c \
        $(LOCAL_PATH)/ir_plug_sdk_colormap_rgb888.c \
        $(LOCAL_PATH)/ir_plug_sdk_font_8.c \
        $(LOCAL_PATH)/ir_plug_sdk_font_12.c \
        $(LOCAL_PATH)/ir_plug_sdk_glib.c \
        $(LOCAL_PATH)/ir_plug_sdk_common.c \
        $(LOCAL_PATH)/ir_plug_sdk_tac.c \
        $(LOCAL_PATH)/ir_plug_sdk_tav.c \
        $(LOCAL_PATH)/ir_plug_sdk_tuc.c \
        $(LOCAL_PATH)/ir_plug_sdk_tuv.c \
        $(LOCAL_PATH)/IrSdk.c

LOCAL_MODULE := IrSdk

include $(BUILD_SHARED_LIBRARY)






