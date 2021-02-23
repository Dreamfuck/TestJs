#ifndef __IR_PLUG_SDK_COMMON_H__
#define __IR_PLUG_SDK_COMMON_H__

#include "ir_platform.h"

#define ImgHeight   120
#define ImgWidth    160
#define FccPeriod   239
#define ACBWTIME    350

#define GAIN_MODE_HIGH     (0)
#define GAIN_MODE_LOW      (1)
#define GAIN_MODE_AUTO     (2)


extern U16 RawData[(ImgHeight + 2)*ImgWidth];
extern S16 UniformComArray[300][ImgHeight][ImgWidth];       // 均与性校准参数1
extern S16 UniformComArray0[300][ImgHeight][ImgWidth];
extern S16 FccComArray[300];                                // 均与性校准参数2
extern S16 FccComArray0[300];
extern F32 g_IrWindowRe;
extern S32 g_AcuHighLimit;
extern S32 g_AcuLowLimit;
extern F32 g_UniformData;
extern F32 AcuImgSaveBuf[ImgHeight][ImgWidth];   //Used to store the result of accuracy temperature; Treal = Data/20 一帧温度数据
extern S32 AcuImgBuf[ImgHeight][ImgWidth];       //used to display in normal mode 一帧 8bit 数据
extern S32 DefaultImgBuf[ImgHeight][ImgWidth];   //used to display in calibrate mode 校准/验证模式下的 8bit 数据
extern S32 g_FrameBufferRgb[ImgWidth][ImgHeight];// 32bit rgb888 数据
extern F32 g_FrameBufferTemp[240][320];         // 放大后的温度矩阵
extern U16 CalEnPoint;

extern F32 g_CalibrateCoef_A[13];    // 精度校准参数1
extern F32 g_CalibrateCoef_B[13];    // 精度校准参数2
extern S32 g_GetRawData[13];         // 精度校准参数3
extern S32 g_TargetTemp[13];


extern S32 FilteredData;
extern S32 GausTemp[3][3];
extern S32 AcuGausTemp[3][3];

extern F32 AcOffsetGrantLow[13];
extern F32 AcOffsetGrantHigh[13];
extern F32 AvOffsetGrantLow[13];
extern F32 AvOffsetGrantHigh[13];

extern S32 RawBufForTfCorrect[ImgHeight * ImgWidth];
extern S32 RawBufForGausfillter[ImgHeight][ImgWidth];
extern S32 RawBufForCal[ImgHeight][ImgWidth];
extern S32 RawdataBuf[ImgHeight][ImgWidth];
extern void ImageEnhance(S32 ImgSrc[][ImgWidth]);
extern char g_StringBuffer[1024];
extern U08 g_LeptonCalibrating;
extern U08 g_TempValueStable;
extern F32 g_TempRange;
extern S32 g_BootTime;
extern S32 g_Fcctime;
extern F32 VerTempArray[6];
extern F32 g_Gain;
extern S32 g_FilterSelected;
extern F32 g_Emissivity;

typedef enum {
    FPA_TEMP_MODE_RAW = 0x03,
    FPA_TEMP_MODE_MANUAL,
    FPA_TEMP_MODE_NUM
};
extern VU08 g_FpaTempMode;
extern F32 g_CurrentFpaTemp;

extern U08 GainMode;
extern U08 LeptonId;

typedef struct
{
    F32 R;
    F32 B;
    F32 O;
    F32 F;
} PlankPar;

extern S32 GainSet(F32 Gain);
extern S32 FilterEnable(S32 En);
extern S32 DefaultTemp2Raw(F32 KelvinTemp);          //
extern F32 DefaultRaw2Temp(S32 Raw);                 //
extern S32 GetLeptonBootTime(U16 *RawData);          //
extern S32 GetLeptonFccTime(U16 *RawData);           //
extern F32 GetFccFpaTemp(U16 *RawData);              //
extern F32 AccuracyRaw2Temp(S32 RawValue);           //
extern S32 AcuImgCalc(F32);                             //image get and process in normal mode, return the processed image in a certain buffer
extern S32 DefaultImgCalc();                         //

extern void ImageZoom(F32 *srcBuffer, U16 srcW, U16 srcH, F32 *dstBuffer, U08 scale);
extern void ImageRotate90(F32 *src, F32 *dst);
extern S32 AccuracyTemp2Raw(F32 Temp);

#endif