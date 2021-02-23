#include "ir_plug_sdk_algorithm.h"
#include "ir_plug_sdk_colormap.h"
#include "ir_plug_sdk_common.h"



/* Private types ---------------------------------------------------------------*/

/* Private macros --------------------------------------------------------------*/
#define HistableNum             (2050)


/* Private variables -----------------------------------------------------------*/
/* lepton 原始数据处理后的 8-bit 数据缓冲区                                     */
static U08 g_DataBufferAgc[121 * 161] = {0};


/* IR图像相关参数定义                                   */
static U16 g_IrImageWidth       = 0;
static U16 g_IrImageHeight      = 0;
static U32 g_IrImageSize        = 0;
static U32 g_IrImageByteSize    = 0;

static U16* g_pIrImageRaw = 0;
static S32* g_pIrImageRawEx = 0;

/* 温度表                                           */
static U16 gs_TEMP_TABLE[16384] = {0};

/* 直方图统计表                                     */
static U16 g_HisTable[HistableNum] = {0};
static U32 g_Sum_HisTable[HistableNum] = {0};


/*                                                  */
static S16 CalComData[296] =
    {
        77, 77, 76, 75, 73, 72, 71, 70, 68, 67, 66, 65, 63, 61, 59, 56, 54, 52, 50, 48,
        46, 45, 42, 40, 39, 37, 36, 34, 33, 31, 28, 26,  24, 21, 18, 16, 13, 10, 8, 4, 1,
        -1, -4, -4, -4, -4, -5, -6, -7, -9, -12, -14, -15, -17, -19, -19, -21, -21, -22,
        -22, -22, -22, -22, -23,  -24, -26, -28, -29, -32, -32, -31, -31, -31, -31, -33,
        -34, -35, -36, -36, -35, -35, -34, -34, -34, -34, -34, -34, -34, -34, -33, -33,
        -34, -34, -34, -36, -36,  -37, -39, -41, -42, -43, -44, -44, -43, -43, -42, -41,
        -41, -41, -41, -41, -41, -40, -40, -39, -39, -39, -40, -41, -41, -41, -41, -40,
        -39, -38, -38, -37, -36,  -36, -36, -35, -34, -34, -34, -34, -34, -34, -34, -33,
        -33, -32, -31, -30, -30, -29, -29, -30, -30, -30, -29, -29, -28, -28, -28, -29,
        -29, -28, -27, -27, -27,  -28, -29, -30, -30, -30, -29, -28, -28, -28, -28, -28,
        -28, -27, -27, -26, -26, -26, -25, -24, -23, -22, -22, -23, -24, -25, -26, -26,
        -25, -25, -25, -25, -25,  -24, -23, -23, -22, -22, -22, -23, -22, -21, -21, -21,
        -20, -20, -19, -18, -18, -18, -17, -17, -16, -15, -14, -13, -12, -12, -12, -11,
        -11, -11, -11, -11, -12,  -12, -11, -11, -11, -10, -8, -7, -6, -4, -3, -3, -3, -4,
        -4, -5, -5, -7, -7, -6, -5, -4, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -4,  -5,
        -5, -5, -5, -4, -3, -3, -3, -3, -3, -3, -2, -1, 0, 1, 2, 3, 4, 3, 2, 2, 1, 1, 2,
        2, 3, 3, 4, 4, 4, 4, 4,  4, 5, 5, 6, 7, 7, 7, 7
    };

/* Private constants -----------------------------------------------------------*/

/* Private functions -----------------------------------------------------------*/
static void ImageLutCalc(void);
static void Lepton_Dprocess_HotMetal(void);
static F32 AcuTempCalc(F32 AtmTemp, F32 BkgTemp, U16 Distance, F32 Em,
                       F32 FpaTemp, U16 TargetTempRaw);
static U16 Temp2Raw(F32 KelvinTemp);
static F32 Raw2Temp(U16 RawData);

/* Exported variables ----------------------------------------------------------*/

/* 当前 lepton 的测温参数结构体，
 * 每次读取图像数据前，必须首先获取当前测试参数     */
ExtraInfo g_ExtralInfo;

/* lepton 原始数据缓冲区，每次读取数据都存储到该缓冲区后再做处理                */
U16 g_DataBufferRaw[120 * 160] = {0};



/* Exported functions ----------------------------------------------------------*/
void IR_ALG_Init(U16 width, U16 height)
{
  g_IrImageWidth      = width;
  g_IrImageHeight     = height;
  g_IrImageSize       = width * height;
  g_IrImageByteSize   = g_IrImageSize * 2;

  //g_pIrImageRaw = g_DataBufferRaw;
  g_pIrImageRawEx = (S32*)AcuImgBuf;
}


void IR_ALG_ImageProcess(U16* pRawBuffer)
{

}









F32 IR_ALG_TempCalc(U16 RawData)
{
  U16 RawDataTemp;
  F32 AtmTemp, BkgTemp, Em, FpaTemp;
  F32 fResult;
  RawDataTemp = RawData + g_ExtralInfo.rowComData;
  AtmTemp = (F32)g_ExtralInfo.atmTemp + 273.15f;
  BkgTemp = (F32)g_ExtralInfo.bkgTemp / 10.0f;
  Em = g_ExtralInfo.emissivity;
  FpaTemp = (F32)g_ExtralInfo.fpaTemp / 100.0f;

  fResult = AcuTempCalc(AtmTemp, BkgTemp,
                        g_ExtralInfo.Distance, Em, FpaTemp, RawDataTemp);

  fResult += g_ExtralInfo.tempAddVal;
  return (fResult - 273.0f);
}

void IR_ALG_ConvertMapRgb565( U16 * pRgb565Buffer )
{
  int pos= 0;

  for (int row = 0; row < g_IrImageHeight; ++row)
  {
    for (int col = 0; col < g_IrImageWidth; ++col)
    {
      pos = row * (g_IrImageWidth + 1) + col;

      pRgb565Buffer[row * g_IrImageWidth + col] =
          ColorMap_Color_RGB565(g_pIrImageRawEx[pos]);
    }
  }
}

void IR_ALG_ConvertMapRgb888(U32 *pRgb888Buffer)
{
  int pos = 0, dst = 0;
  int row = 0, col = 0;

  for (row = 0; row < ImgHeight; ++row)
  {
    for (col = 0; col < ImgWidth; ++col)
    {
      pos = row * ImgWidth + col;
//      dst = col * ImgHeight + row;
        dst = (col) * ImgHeight + (ImgHeight - row - 1);
      pRgb888Buffer[dst] = ColorMap_Color_RGB888(AcuImgBuf[row][col]);
    }
  }
}


// 辐射率计算温度值公式:  {[(K^4 - FPA^4) / E] + FPA^4}^0.25 - 273.15
// K 为目标开尔文温度值，FPA环境温度，E为辐射率
void IR_ALG_ConvertTemperature(F32 * pTempBuffer)
{
    int pos = 0, dst = 0;
    int row = 0, col = 0;

    F32 fpaTemp = 0.0f;
    F32 emissivity = g_Emissivity / 0.95f;
    F32 theTemp = 0.0f;
    U08 tempOverflow = 0;

    // 根据不同模式，计算当前环境温度值
    if (FPA_TEMP_MODE_MANUAL == g_FpaTempMode)
    {
        fpaTemp = g_CurrentFpaTemp;
    }
    else
    {
        fpaTemp = GetFccFpaTemp(RawData);
    }

    for (row = 0; row < ImgHeight; ++row)
    {
        for (col = 0; col < ImgWidth; ++col)
        {
            pos = row * ImgWidth + col;
//            dst = col * ImgHeight + row;
            dst = (col) * ImgHeight + (ImgHeight - row - 1);
            theTemp = pow((pow((F32) (AcuImgSaveBuf[row][col] + 273.15f), 4) - pow(fpaTemp, 4))
                          / emissivity + pow(fpaTemp, 4), 0.25) - 273.15f;
            if (theTemp < 0.0f)
            {
                tempOverflow = 1;
                break;
            }

            pTempBuffer[dst] = theTemp;
        }
        if (1 == tempOverflow)
        {
            break;
        }
    }

  if (1 == tempOverflow)
  {
      for (row = 0; row < ImgHeight; ++row)
      {
          for (col = 0; col < ImgWidth; ++col)
          {
              pos = row * ImgWidth + col;
//              dst = col * ImgHeight + row;
              dst = (col) * ImgHeight + (ImgHeight - row - 1);
              pTempBuffer[dst] = AcuImgSaveBuf[row][col];
          }
      }
  }
}

void IR_ALG_Colormap( U08 type )
{
  IR_COLORMAP_Set((IR_PLUG_SDK_ColormapTypeDef)type);
}

F32 AtmEmCalc(U16 Distance)
{
  F32 Eatm;
  Eatm = (Distance - 600.0f) * 0.015f / 300.0f;
  return Eatm;
}

F32 CalRaw(U16 RawData)
{
  F32 fResult;

  if (RawData < g_ExtralInfo.StPoint[1])
  {
    fResult = ((F32)RawData) * g_ExtralInfo.AcuCalibrate_ra[0] +
        g_ExtralInfo.AcuCalibrate_rb[0];
  }
  else if ((RawData >= g_ExtralInfo.StPoint[1])
      && (RawData < g_ExtralInfo.StPoint[2]))
  {
    fResult = ((F32)RawData) * g_ExtralInfo.AcuCalibrate_ra[1] +
        g_ExtralInfo.AcuCalibrate_rb[1];
  }
  else if ((RawData >= g_ExtralInfo.StPoint[2])
      && (RawData < g_ExtralInfo.StPoint[3]))
  {
    fResult = ((F32)RawData) * g_ExtralInfo.AcuCalibrate_ra[2] +
        g_ExtralInfo.AcuCalibrate_rb[2];
  }
  else
  {
    fResult = ((F32)RawData) * g_ExtralInfo.AcuCalibrate_ra[3] +
        g_ExtralInfo.AcuCalibrate_rb[3];
  }

  return fResult;
}


F32 CalRawConvert(U16 RawData)
{
  F32 fResult;

  if (RawData < g_ExtralInfo.TarPoint[1])
  {
    fResult = (((F32)RawData) - g_ExtralInfo.AcuCalibrate_rb[0])
        / g_ExtralInfo.AcuCalibrate_ra[0];
  }
  else if ((RawData >= g_ExtralInfo.TarPoint[1])
      && (RawData < g_ExtralInfo.TarPoint[2]))
  {
    fResult = (((F32)RawData) - g_ExtralInfo.AcuCalibrate_rb[1])
        / g_ExtralInfo.AcuCalibrate_ra[1];
  }
  else if ((RawData >= g_ExtralInfo.TarPoint[2])
      && (RawData < g_ExtralInfo.TarPoint[3]))
  {
    fResult = (((F32)RawData) - g_ExtralInfo.AcuCalibrate_rb[2]) /
        g_ExtralInfo.AcuCalibrate_ra[2];
  }
  else
  {
    fResult = (((F32)RawData) - g_ExtralInfo.AcuCalibrate_rb[3]) /
        g_ExtralInfo.AcuCalibrate_ra[3];
  }

  if (fResult > 0.5f)
  {
    return fResult;
  }
  else
  {
    return 2000.0f;
  }
}

F32 TempCalcAcu(U16 RawData)
{
  F32 fResult;
  F32 fRawData;
  fRawData = (F32)RawData;

  if (fRawData > g_ExtralInfo.MeasureO)
  {
    fResult = g_ExtralInfo.MeasureB
        / log(g_ExtralInfo.MeasureR / (fRawData - g_ExtralInfo.MeasureO)
                  + g_ExtralInfo.MeasureF);
  }
  else
  {
    fResult = 200.0f;
  }

  return fResult;

}

/* Private functions -------------------------------------------------------*/
static void ImageLutCalc(void)
{
  S32 i;
  S32 IntICO;
  F32 Dindex;
  F32 Dtemp;
  F32 ICR = g_ExtralInfo.ImgR;
  F32 ICO = g_ExtralInfo.ImgO;
  F32 ICF = g_ExtralInfo.ImgF;
  F32 ICB = g_ExtralInfo.ImgB;
  IntICO = (S32)ICO + 200;

  for (i = 0; i < 16384; i++)
  {
    Dindex = (F32)i;

    if (i <= IntICO)
    {
      gs_TEMP_TABLE[i] = 0;
    }
    else
    {
      Dtemp = 10.0f * ICB / log(ICR / (Dindex - ICO) + ICF);
      gs_TEMP_TABLE[i] = (U16)Dtemp;
    }
  }
}

static void Lepton_Dprocess_HotMetal(void)
{
  U16 i = 0, j = 0, pos = 0;
  U16 max_sum = 0;
  U16 min_sum = 0;
  U16 max_data = 0;
  U16 min_data = 0;
  U16 alladdnum = 0;
  U08  addval = 30;
  U16 addnum = 500;
  U16 clip_high = 500;
  U32 temp_Cum = 0;

  for (i = HistableNum - 1; i > 0; i--)
  {
    max_sum = max_sum + g_HisTable[i];

    if (max_sum > 0)
    {
      max_data = i;
      break;
    }
  }

  for (i = 0; i < HistableNum; i++)
  {
    min_sum = min_sum + g_HisTable[i];

    if (min_sum > 0)
    {
      min_data = i;
      break;
    }
  }

  if ((max_data - min_data) < 5)
  {
    addval = 100;
    alladdnum = 200;
  }
  else if ((max_data - min_data) < 15)
  {
    addval = 105 - (max_data - min_data);
    alladdnum = 200;
  }
  else if ((max_data - min_data) > 105)
  {
    addval = 20;
    alladdnum = 200;
  }
  else
  {
    addval = 125 - (max_data - min_data);
    alladdnum = 200;
  }

  for (i = 0; i < min_data; i++)
  {
    g_Sum_HisTable[i] = (addnum + alladdnum) * addval / 5; //5120;
  }

  temp_Cum = (addnum + alladdnum) * addval / 5;

  for (i = min_data; i <= max_data; i++)
  {
    if (g_HisTable[i] > clip_high)
    {
      g_HisTable[i] = addnum + alladdnum + clip_high;//1024;
    }
    else if (g_HisTable[i] > 0)
    {
      g_HisTable[i] += addnum + alladdnum;
    }
    else
    {
      g_HisTable[i] += alladdnum;
    }

    temp_Cum += g_HisTable[i];
    g_Sum_HisTable[i] += temp_Cum;
  }

  for (i = HistableNum - 1; i > max_data; i--)
  {
    g_Sum_HisTable[i] = g_Sum_HisTable[max_data];
  }

  /* 将原始 14bit 数据转换为 8bit 数据                */
  for (i = 0; i < g_IrImageHeight; i++)
  {
    for (j = 0; j < g_IrImageWidth; j++)
    {
      *(g_pIrImageRawEx + i * (g_IrImageWidth + 1) + j) =
          255 * (g_Sum_HisTable[g_pIrImageRaw[i * g_IrImageWidth + j]])
              / (g_Sum_HisTable[max_data] + addnum * addval);
    }
  }

  /* 多填充一列数据                           */
  for (i = 0; i < g_IrImageHeight; i++)
  {
    pos = i * (g_IrImageWidth + 1) + g_IrImageWidth;
    g_pIrImageRawEx[pos] = g_pIrImageRawEx[pos - 1];
  }

  /* 多填充一行数据                           */
  for (j = 0; j < (g_IrImageWidth + 1); j++)
  {
    g_pIrImageRawEx[g_IrImageHeight * (g_IrImageWidth + 1) + j] =
        g_pIrImageRawEx[(g_IrImageHeight - 1) * (g_IrImageWidth + 1) + j];
  }
}



F32 AcuTempCalc(F32 AtmTemp, F32 BkgTemp, U16 Distance, F32 Em,
                F32 FpaTemp, U16 TargetTempRaw)
{
  U16 Watm;
  U16 Wbkg;
  U16 Wwin;

  F32 fWatm, fWbkg, fWwin;
  F32 Eatm;
  F32 fGetWtarget;
  F32 fWtarget;
  U16 Result;
  F32 fResult;

  Watm = Temp2Raw(AtmTemp);
  fWatm = CalRaw(Watm);     //convert standard value to real data

  //  if((BkgTemp >= 2230) && (BkgTemp < 4230))           //-50 ~ 150
  //      Wbkg = TempToWraw[BkgTemp + 500 - 2730];
  //  else
  //      Wbkg = TempToWraw[1999];
  Wbkg = Temp2Raw(BkgTemp);
  fWbkg = CalRaw(Wbkg);

  //  if((FpaTempRaw/10 >= 2230) && (FpaTempRaw/10 < 4230))
  //      Wwin = TempToWraw[FpaTempRaw/10 + 500 - 2730];
  //  else
  //      Wwin = TempToWraw[1999];
  Wwin = Temp2Raw(FpaTemp);
  fWwin = CalRaw(Wwin);



  //Atm Emissivity
  Eatm = AtmEmCalc(Distance);
  //Erwin represent Emissivity rate of the IR window
  //Em represent Emissivity rate of the target
  fGetWtarget = (F32)TargetTempRaw;
  fWtarget = fGetWtarget / (1.0f - g_ExtralInfo.Erwin) / (1.0f - Eatm) / Em -
      (1.0f - Em) * fWbkg / Em - Eatm * fWatm / (1.0f - Eatm) / Em -
      g_ExtralInfo.Erwin * fWwin / (1.0f - g_ExtralInfo.Erwin) / (1.0f - Eatm) / Em;

  //map to standard lookup table value
  Result = (U16)CalRawConvert((U16)fWtarget);

  //lookup table temperature calc
  fResult = TempCalcAcu(Result);


  return fResult;
}



U16 Temp2Raw(F32 KelvinTemp)
{
  U16 Result;
  F32 fResult;
  fResult = g_ExtralInfo.MeasureR
      / (exp(g_ExtralInfo.MeasureB / KelvinTemp)
          - g_ExtralInfo.MeasureF) + g_ExtralInfo.MeasureO;
  Result = (U16)fResult;
  return Result;
}

F32 Raw2Temp(U16 RawData)
{
  F32 fResult;
  F32 fRawData;

  fRawData = (F32)RawData;

  if (fRawData > g_ExtralInfo.MeasureO)
  {
    fResult = g_ExtralInfo.MeasureB
        / log(g_ExtralInfo.MeasureR / (fRawData - g_ExtralInfo.MeasureO)
                  + g_ExtralInfo.MeasureF);
  }
  else
  {
    fResult = 200.0f;
  }

  return fResult;
}












