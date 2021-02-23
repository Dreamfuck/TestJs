//
// Created by E001 on 17/6/14 014.
//
#include <jni.h>

#ifndef NDK_IRSDK_H
#define NDK_IRSDK_H


#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     cn_edu_swust_ndk_IrSdk
 * Method:    IrSdkInit
 * Signature: (Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_net_launchdigital_irsdk_IrSdk_IrSdkInit
        (JNIEnv *, jobject, jstring, jint, jint, jint);

/*
 * Class:     cn_edu_swust_ndk_IrSdk
 * Method:    IrSdkReadFrameRgb
 * Signature: ([SI)I
 */
JNIEXPORT jint JNICALL Java_net_launchdigital_irsdk_IrSdk_IrSdkReadFrameRgb
        (JNIEnv *, jobject, jintArray, jint);


JNIEXPORT jint JNICALL Java_net_launchdigital_irsdk_IrSdk_IrSdkReadFrameRaw
        (JNIEnv *, jobject, jshortArray, jint);



#ifdef __cplusplus
}
#endif




#endif //NDK_IRSDK_H
