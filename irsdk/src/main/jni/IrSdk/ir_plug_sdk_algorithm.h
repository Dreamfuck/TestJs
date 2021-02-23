#pragma once
#include "ir_platform.h"




/**
  * @brief  与测温相关的结构体定义
  */
typedef struct
{
    U16     atmTemp;        /*!< 大气温度                       */
    U16     bkgTemp;        /*!< 背景温度                       */
    U16     Distance;//测量距离
    U16     fpaTemp;//fpa温度
    F32     emissivity;//辐射率
    F32     Erwin;
    F32     MeasureB;
    F32     MeasureF;
    F32     MeasureR;
    F32     MeasureO;
    F32     ImgB;
    F32     ImgF;
    F32     ImgO;
    F32     ImgR;
    F32     AcuCalibrate_ra[4];
    F32     AcuCalibrate_rb[4];
    U16     StPoint[5];
    U16     TarPoint[5];
    S16     rowComData;
    F32     tempAddVal;
    U08     equipVersion;
    U08     cameraVersion;
} ExtraInfo;



/**
  * @brief  IR 算法操作接口
  */
typedef struct
{
  void (*Init)(U16, U16);
  void (*Process)(U16*);
  F32  (*Temp)(U16);
  void (*Rgb)(U32*);
  void (*Colormap)(U08);
}IR_ALGORITHM_HandleDef;



extern ExtraInfo g_ExtralInfo;
extern U16 g_DataBufferRaw[];

void IR_ALG_Init(U16 width, U16 height);
void IR_ALG_ImageProcess(U16 *pRawBuffer);
void IR_ALG_ConvertMapRgb565(U16 * pRgb565Buffer);
void IR_ALG_ConvertMapRgb888(U32 * pRgb888Buffer);
void IR_ALG_ConvertTemperature(F32 * pTempBuffer);
void IR_ALG_Colormap(U08 type);
F32 IR_ALG_TempCalc(U16 RawData);




