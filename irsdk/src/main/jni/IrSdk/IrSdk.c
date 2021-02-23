//
// Created by E001 on 17/6/14 014.
//
//#include <libusb/libusb.h>
#include "IrSdk.h"
#include "../libusb/libusb/libusb.h"

#include "ir_plug_device.h"
#include "ir_plug_sdk.h"
#include "ir_plug_sdk_common.h"

#include <stdio.h>
#include <stdlib.h>


/* 设备 ID 信息                 */
#define IR_PLUG_DEV_VID             (0x0483)
#define IR_PLUG_DEV_PID             (0x8080)//(0x1234)

/* USB 端点、配置和接口信息           */
#define IR_PLUG_DEV_CONFIG          (1)
#define IR_PLUG_DEV_INTERFACE       (0)
#define IR_PLUG_DEV_BULK_IN         (0x81)
#define IR_PLUG_DEV_BULK_OUT        (0x01)

#if 0
/**
  * @brief USB 操作返回结果
  */
typedef enum
{
    IR_PLUG_DEV_SUCCESS    = -100,       /*!< 成功               */
    IR_PLUG_DEV_INVALID_HANDLE,          /*!< 提供的句柄无效     */
    IR_PLUG_DEV_USBCORE,                 /*!< USB内核错误        */
} IR_PLUG_DEV_ErrorTypeDef;
#endif

libusb_context *g_pLibUsbContext;
libusb_device *g_pLibUsbDevice = NULL;
libusb_device_handle *g_pLibUsbDeviceHandle = NULL;
static char g_strBuff[1024] = {0};

JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkInit(JNIEnv *env, jobject instance, jstring deviceName_,
        jint deviceFileDescriptor, jint busNum, jint devNum)
{
    const char *deviceName = (*env)->GetStringUTFChars(env, deviceName_, 0);

    int ret = 0;

    if (IR_PLUG_SDK_ERR_SUCCESS != IR_PLUG_SDK_Init(IR_PLUG_TYPE_3_0,
            deviceName,
            deviceFileDescriptor,
            busNum,
            devNum))
    {
        ret = -1;
    }

#if 0
    // TODO

    if (0 != libusb_init(&g_pLibUsbContext))
    {
        return (IR_PLUG_DEV_USBCORE);
    }

    g_pLibUsbDevice = libusb_get_device2(g_pLibUsbContext, deviceName);

    if (NULL == g_pLibUsbDevice)
    {
        return (IR_PLUG_DEV_USBCORE);
    }

    if (0 != libusb_open2(g_pLibUsbDevice, &g_pLibUsbDeviceHandle, deviceFileDescriptor))
    {
        return (IR_PLUG_DEV_USBCORE);
    }


    /* 启用设备指定的 配置 和 接口 */
    if (libusb_set_configuration(g_pLibUsbDeviceHandle, IR_PLUG_DEV_CONFIG) < 0 ||
            libusb_claim_interface(g_pLibUsbDeviceHandle, IR_PLUG_DEV_INTERFACE) < 0)
    {
        libusb_close(g_pLibUsbDeviceHandle);
        return (IR_PLUG_DEV_USBCORE);
    }

#endif

    (*env)->ReleaseStringUTFChars(env, deviceName_, deviceName);

    return (ret);
}

JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkReadFrameRgb(JNIEnv *env,
        jobject instance,
        jintArray buffer_,
        jint size)
{
    jint *buffer = (*env)->GetIntArrayElements(env, buffer_, NULL);

    if (IR_PLUG_SDK_ERR_SUCCESS != IR_PLUG_SDK_ReadImageRgb((U32 *) buffer, (U32) size))
    {
        return (-1);
    }

    (*env)->ReleaseIntArrayElements(env, buffer_, buffer, 0);

    if (1 == g_LeptonCalibrating)
    {
        return (1);
    }
    else
    {
        return (0);
    }
}

JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkReadFrameRaw(JNIEnv *env,
        jobject instance,
        jshortArray buffer_,
        jint size)
{
    jshort *buffer = (*env)->GetShortArrayElements(env, buffer_, NULL);

    // TODO
    if (NULL == g_pLibUsbDeviceHandle)
    {
        return (-1);
    }

    int ret = 0;
    //    size = 120 * 160 * 2;



    /* 发送读取数据请求                         */
    ret = libusb_control_transfer(g_pLibUsbDeviceHandle,
                                  LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS
                                  | LIBUSB_RECIPIENT_ENDPOINT,
                                  0x10,
                                  0x00,
                                  0x00, NULL, 0, 1000);

    if (ret < 0)
    {
        snprintf(g_strBuff, sizeof(g_strBuff), "%s", libusb_error_name(ret));
        return (IR_PLUG_DEV_USBCORE);
    }


    /* 通过批量传输读取一帧数据                 */
    int bulkOLen = 0;
    ret = libusb_bulk_transfer(g_pLibUsbDeviceHandle, IR_PLUG_DEV_BULK_IN,
                               (unsigned char *) buffer, size, &bulkOLen, 5000);

    if (ret < 0)
    {
        snprintf(g_strBuff, sizeof(g_strBuff), "%s", libusb_error_name(ret));
        return (IR_PLUG_DEV_USBCORE);
    }

    (*env)->ReleaseShortArrayElements(env, buffer_, buffer, 0);

    return (0);
}

JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkDeinit(JNIEnv *env, jobject instance)
{

    // TODO
    if (IR_PLUG_SDK_ERR_SUCCESS != IR_PLUG_SDK_Deinit())
    {
        return (-1);
    }

    return (0);
}

JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkCaptureFrame(JNIEnv *env, jobject instance,
        jintArray imgBuffer_, jint imgBuffSize,
        jdoubleArray tempBuffer_, jint tempBuffSize)
{
    jint *imgBuffer = (*env)->GetIntArrayElements(env, imgBuffer_, NULL);
    jdouble *tempBuffer = (*env)->GetDoubleArrayElements(env, tempBuffer_, NULL);

    // TODO
    int ret = 0;
    ret = (int) IR_PLUG_SDK_Capture((U32 *) imgBuffer,
                                    (U32) imgBuffSize,
                                    (F32 *) tempBuffer,
                                    (U32) tempBuffSize);

    (*env)->ReleaseIntArrayElements(env, imgBuffer_, imgBuffer, 0);
    (*env)->ReleaseDoubleArrayElements(env, tempBuffer_, tempBuffer, 0);

    if (0 == ret)
    {
        // bit0 表示是否成功，1 成功
        ret |= 0x01;

        // bit1 表示 lepton 是否正在校准
        if (1 == g_LeptonCalibrating)
        {
            ret |= (0x01 << 1);
        }

        // bit2 表示 当前温度是否稳定
        if (1 == g_TempValueStable)
        {
            ret |= (0x01 << 2);
        }

        return (ret);
    }
    else
    {
        ret = 0;

        return (ret);
    }
}

JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkSetWorkDir(JNIEnv *env, jobject instance, jstring dir_)
{
    const char *dir = (*env)->GetStringUTFChars(env, dir_, 0);

    // TODO
    int ret = Ir_PLUG_SDK_SetWorkDir(dir);

    (*env)->ReleaseStringUTFChars(env, dir_, dir);

    return (ret);
}

JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkGetDeviceInfo(JNIEnv *env,
        jobject instance,
        jobject deviceInfo)
{

    int ret = 0;

    // TODO
    IR_PLUG_SDK_DeviceInfoTypeDef cDevInfo;

    ret = IR_PLUG_SDK_GetDeviceInfo(&cDevInfo);

    if (IR_PLUG_SDK_ERR_SUCCESS != ret)
    {
        return (-1);
    }

    jclass clazz;
    jfieldID fid;

    clazz = (*env)->GetObjectClass(env, deviceInfo);

    if (0 == clazz)
    {
        return (-1);
    }

    jstring strTemp;


    // public String sdkVersion;       /*!< 当前 SDK 版本信息，格式 x.x.x */
    fid = (*env)->GetFieldID(env, clazz, "sdkVersion", "Ljava/lang/String;");
    strTemp = (*env)->NewStringUTF(env, cDevInfo.sdkVersion);
    (*env)->SetObjectField(env, deviceInfo, fid, strTemp);

    // public int  tacParamValid;             /*!< TAC 校准参数是否有效 */
     fid = (*env)->GetFieldID(env, clazz, "tacParamValid", "I");
     (*env)->SetIntField(env, deviceInfo, fid, cDevInfo.tacParamValid);


    //public String tacDatetime;           /*!< TAC校准日期时间20170719 180101              */
    fid = (*env)->GetFieldID(env, clazz, "tacDatetime", "Ljava/lang/String;");
    strTemp = (*env)->NewStringUTF(env, cDevInfo.tacDatetime);
    (*env)->SetObjectField(env, deviceInfo, fid, strTemp);

//    public int  tucParamValid;             /*!< TUC 校准参数是否有效 */
    fid = (*env)->GetFieldID(env, clazz, "tucParamValid", "I");
    (*env)->SetIntField(env, deviceInfo, fid, cDevInfo.tucParamValid);

//    public String tucDatetime;           /*!< TUC校准日期时间20170719 180101              */
    fid = (*env)->GetFieldID(env, clazz, "tucDatetime", "Ljava/lang/String;");
    strTemp = (*env)->NewStringUTF(env, cDevInfo.tucDatetime);
    (*env)->SetObjectField(env, deviceInfo, fid, strTemp);

//    public int hardwareInfoSource;         /*!< 模组硬件信息来源 */
    fid = (*env)->GetFieldID(env, clazz, "hardwareInfoSource", "I");
    (*env)->SetIntField(env, deviceInfo, fid, cDevInfo.hardwareInfoSource);

//
//    /*!< 模组硬件相关信息 */
//    public int  leptonType;            // Lepton 型号
    fid = (*env)->GetFieldID(env, clazz, "leptonType", "I");
    (*env)->SetIntField(env, deviceInfo, fid, cDevInfo.hardwareInfo.leptonType);

//    public String leptonId;          // Lepton 序列号
    fid = (*env)->GetFieldID(env, clazz, "leptonId", "Ljava/lang/String;");
    strTemp = (*env)->NewStringUTF(env, cDevInfo.hardwareInfo.leptonId);
    (*env)->SetObjectField(env, deviceInfo, fid, strTemp);

//    public String firmwareVersion;    // 固件版本号：1.0.0，2.0.1等
    fid = (*env)->GetFieldID(env, clazz, "firmwareVersion", "Ljava/lang/String;");
    strTemp = (*env)->NewStringUTF(env, cDevInfo.hardwareInfo.firmwareVersion);
    (*env)->SetObjectField(env, deviceInfo, fid, strTemp);

//    public String hardwareId;        // CPU ID
    fid = (*env)->GetFieldID(env, clazz, "hardwareId", "Ljava/lang/String;");
    strTemp = (*env)->NewStringUTF(env, cDevInfo.hardwareInfo.hardwareId);
    (*env)->SetObjectField(env, deviceInfo, fid, strTemp);

    return (ret);

}

JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkGetDeviceState(JNIEnv *env, jobject instance)
{

    // TODO
    U08 state = 0xFF;

    IR_PLUG_SDK_GetDeviceState(&state);

    return (state);

}


JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkEnterTempAccuracyCalibrationMode(JNIEnv *env,
        jobject instance)
{

    // TODO
    if (IR_PLUG_SDK_ERR_SUCCESS != IR_PLUG_SDK_EnterTempAccuracyCalibrationMode())
    {
        return (-1);
    }
    else
    {
        return (0);
    }
}
JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkAbortTempAccuracyCalibrationMode(JNIEnv *env,
        jobject instance)
{

    // TODO
    if (IR_PLUG_SDK_ERR_SUCCESS != IR_PLUG_SDK_AbortTempAccuracyCalibrationMode())
    {
        return (-1);
    }
    else
    {
        return (0);
    }


}
JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkEnterTempUniformityCalibrationMode(JNIEnv *env,
        jobject instance)
{
    // TODO
    if (IR_PLUG_SDK_ERR_SUCCESS != IR_PLUG_SDK_EnterTempUniformityCalibrationMode())
    {
        return (-1);
    }
    else
    {
        return (0);
    }
}
JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkAbortTempUniformityCalibrationMode(JNIEnv *env,
        jobject instance)
{

    if (IR_PLUG_SDK_ERR_SUCCESS != IR_PLUG_SDK_AbortTempUniformityCalibrationMode())
    {
        return (-1);
    }
    else
    {
        return (0);
    }
}


JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkStartTacCapture(JNIEnv *env, jobject instance)
{

    // TODO

    IR_PLUG_SDK_StartTacCapture();

    return (0);

}

JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkEnableDebug(JNIEnv *env, jobject instance, jint enable)
{
    // TODO
    return IR_PLUG_SDK_EnableDebug(enable);
}

JNIEXPORT jdouble JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkGetCurrentTemp(JNIEnv *env,
        jobject instance,
        jint x,
        jint y)
{
    // TODO
    double temp = 0.0f;

    if (IR_PLUG_SDK_ERR_SUCCESS == IR_PLUG_SDK_GetTemp(x, y, &temp))
    {
        return (temp);
    }
    else
    {
        return (0.0f);
    }
}

JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkSetLeptonType(JNIEnv *env, jobject instance, jint type) {

    // TODO
    return IR_PLUG_SDK_SetLeptonType((U08)type);
}

JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkShutterCalibration(JNIEnv *env, jobject instance) {

    // TODO
    int ret = 0;

    if (IR_PLUG_SDK_ERR_SUCCESS != IR_PLUG_SDK_ShutterCalibration()) {
        ret = -1;
    }

    return (ret);

}

JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkSetColormap(JNIEnv *env, jobject instance, jint type) {

    // TODO
    return (IR_PLUG_SDK_SetPallete(type));
}

JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkSetFlashingPeriod(JNIEnv *env, jobject instance,
                                                          jbyte onSec, jbyte offSec) {
    // TODO
    return (IR_PLUG_SDK_SetFlashingPeriod(onSec, offSec));
}

JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkSetEmissivity(JNIEnv *env, jobject instance,
                                                      jdouble emissivity) {

    // TODO
    int ret = 0;
    if (IR_PLUG_SDK_ERR_SUCCESS != IR_PLUG_SDK_SetEmissivity(emissivity)) {
        ret = -1;
    } else {
        ret = 0;
    }

    return (ret);
}

JNIEXPORT jint JNICALL
Java_net_launchdigital_irsdk_IrSdk_IrSdkSetFpaTemp(JNIEnv *env, jobject instance, jdouble fpaTemp,
                                                   jint enable) {
    // TODO
    int ret = 0;
    if (IR_PLUG_SDK_ERR_SUCCESS != IR_PLUG_SDK_SetFpaTemp(fpaTemp, (U32)enable))
    {
        ret = -1;
    }
    else
    {
        ret = 0;
    }

    return (ret);
}