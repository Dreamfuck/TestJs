#include "ir_platform.h"
#include "ir_plug_sdk_common.h"
#include "ir_plug_sdk.h"







#define IMGLTEMP (-20*20)
#define IMGHTEMP (400*20)
#define HistNum  (IMGHTEMP-IMGLTEMP)
S32 HistTable[HistNum] = {0};
F32 HistTabTemp[HistNum] = {0};

U16 RawData[(ImgHeight + 2) * ImgWidth];
S16 UniformComArray[300][ImgHeight][ImgWidth];
S16 UniformComArray0[300][ImgHeight][ImgWidth];
S32 RawdataBuf[ImgHeight][ImgWidth];
S32 OrgImgBuf[ImgHeight][ImgWidth];


S16 FccComArray[300];
S16 FccComArray0[300];
F32 g_IrWindowRe = 0.1f;
S32 g_AcuHighLimit = 720;
S32 g_AcuLowLimit = 476;


F32 g_Gain = 36.0f;
S32 g_FilterSelected = 0;

F32 AcuImgSaveBuf[ImgHeight][ImgWidth];   //Used to store the result of accuracy temperature; Treal = Data/20 - 273
S32 AcuImgBuf[ImgHeight][ImgWidth];       //used to display in normal mode
S32 AcuImgTemp[ImgHeight][ImgWidth];
S32 DefaultImgBuf[ImgHeight][ImgWidth];   //used to display in calibrate mode
S32 g_FrameBufferRgb[ImgWidth][ImgHeight];
F32 g_FrameBufferTemp[240][320];

/*
    for lepton2.5,3.5,the front 5 points are high gain calibration points
    the last 8 are low gain calibration points
    lepton 2.0 and 3.0 use only front 5 points
*/
F32 g_CalibrateCoef_A[13] = {1.0};
F32 g_CalibrateCoef_B[13] = {0};
S32 g_GetRawData[13] = {5450, 6150, 6850, 7850, 8450, 5450, 6150, 6850, 7850, 8450, 10050, 11450, 13450};
S32 g_TargetTemp[13] = {5460, 6160, 6860, 7860, 8460, 5460, 6160, 6860, 7860, 8460, 10060, 11460, 13460};


S32 FilteredData = 0;
S32 GausTemp[3][3] = {{967, 1176, 967}, {1176, 1428, 1176}, {967, 1176, 967}};
S32 AcuGausTemp[3][3] = {{1, 2, 1}, {2, 6, 2}, {1, 2, 1}};
S32 AdjGausTemp[3][3] = {{ -2, -2, -2}, { -2, 17, -2}, { -2, -2, -2}};
S32 RawBufForTfCorrect[ImgHeight * ImgWidth];
S32 RawBufForGausfillter[ImgHeight][ImgWidth];
S32 RawBufForCal[ImgHeight][ImgWidth];
S32 ImgHisTable[256];
S32 ImgSumHisTable[256];


// 精度校准模式，各个校准点的验证上限、下限
F32 AcOffsetGrantLow[13]  = {5160,5960,6660,7560,8160,5160,5960,6660,7560,8160,9660,11060,12960};
F32 AcOffsetGrantHigh[13] = {5760,6360,7060,8160,8760,5760,6360,7060,8160,8760,10460,11860,13960};
//F32 AvOffsetGrantLow[13]  = {5260,6060,6660,7660,8260,5260,6060,6660,7660,8260,9760,11060,13060};
//F32 AvOffsetGrantHigh[13] = {5660,6260,7060,8060,8660,5660,6260,7060,8060,8660,10360,11860,13860};

//F32 AcOffsetGrantLow[13]  = {5360, 6060, 6760, 7760, 8360, 5360, 6060, 6760, 7760, 8360, 9900, 11300, 13300};
//F32 AcOffsetGrantHigh[13] = {5560, 6260, 6960, 7960, 8560, 5560, 6260, 6960, 7960, 8560, 10220, 11620, 13620};
F32 AvOffsetGrantLow[13]  = {5400, 6120, 6820, 7820, 8400, 5400, 6120, 6820, 7820, 8400, 9960, 11340, 13300};
F32 AvOffsetGrantHigh[13] = {5520, 6200, 6900, 7900, 8520, 5520, 6200, 6900, 7900, 8520, 10160, 11580, 13620};

// F32 AcOffsetGrantLow[13]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// F32 AcOffsetGrantHigh[13] = {15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000};
// F32 AvOffsetGrantLow[13]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// F32 AvOffsetGrantHigh[13] = {15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000};



// 精度验证模式，各个校准点的参考值
//// 25/36/75/30/0
//F32 VerTempArray[6] = {273.15,298.15,309.15,348.15,303.15,273.15};
// "30","36","42","34","24"
F32 VerTempArray[6] = {273.15, 303.15, 309.15, 315.15, 307.15, 297.15};


U16 CalEnPoint = 0;
F32 g_UniformData = 0.0f;
char g_StringBuffer[1024] = {0};


/* 模组的 Lepton 是否正在校准            */
U08 g_LeptonCalibrating = 0;

/* 当前温度是否稳定            */
U08 g_TempValueStable = 0;

/* 当前温宽值                            */
F32 g_TempRange = 9.6f;

/* 当前图像中心点温度值                   */
F32 g_CurrentTemp = 0.0f;

/* 用于调试打印时间信息                   */
S32 g_BootTime = 0;
S32 g_Fcctime = 0;

/* 当前辐射率 */
F32 g_Emissivity = 0.95f;

/* 当前环境温度值模式，
 *      FPA_TEMP_MODE_RAW：通过模组原始数据计算
 *      FPA_TEMP_MODE_MANUAL：通过接口手动设置
 */

VU08 g_FpaTempMode = FPA_TEMP_MODE_RAW;

/* 当前环境温度值 */
F32 g_CurrentFpaTemp = 0.0f;


U08 GainMode = GAIN_MODE_HIGH;
U08 LeptonId = 1;

S32 GainSet(F32 Gain)
{
    g_Gain = Gain;
    return 0;
}
S32 FilterEnable(S32 En)
{
    g_FilterSelected = En;
    return 0;
}



S32 DefaultTemp2Raw(F32 KelvinTemp)
{
    PlankPar LeptonPar;
    F32 temp;
    LeptonPar.B = 1402.1;
    LeptonPar.O = 387.9;
    LeptonPar.R = 334980;
    LeptonPar.F = 3.0;

    temp = LeptonPar.R / (exp(LeptonPar.B / KelvinTemp) - LeptonPar.F) +
           LeptonPar.O;
    return ((S32) temp);
}

F32 DefaultRaw2Temp(S32 Raw)
{
    PlankPar LeptonPar;
    F32 Rawtemp;
    F32 ret;
    LeptonPar.B = 1402.1;
    LeptonPar.O = 387.9;
    LeptonPar.R = 334980;
    LeptonPar.F = 3.0;

    Rawtemp = (F32) Raw;

    if (Rawtemp < LeptonPar.O)
    {
        ret = 0.0;
    }
    else
    {
        ret = LeptonPar.B / log(LeptonPar.R / (Rawtemp - LeptonPar.O) + LeptonPar.F);
    }

    return ret;
}

//////return the time passed since bootup
S32 GetLeptonBootTime(U16 *RawData)
{
    S32 Temp;
    Temp = ((RawData[ImgHeight * ImgWidth + 2 + 1] +
             RawData[ImgHeight * ImgWidth + 2 + 2] * 65536) / 1000);
    return Temp;
}

/////return the time passed since the last fcc
S32 GetLeptonFccTime(U16 *RawData)
{
    S32 Temp;

    Temp = RawData[ImgHeight * ImgWidth + 2 + 1] +
           RawData[ImgHeight * ImgWidth + 2 + 2] * 65536;
    Temp = Temp - (RawData[ImgHeight * ImgWidth + 2 + 30] +
                   RawData[ImgHeight * ImgWidth + 2 + 31] * 65536);

    return (Temp / 1000);
}

U08 GetLeptonCalibrationStatus(U16 *RawData)
{
    U32 status = 0;
    status = RawData[ImgHeight * ImgWidth + 2 + 3]
             | (RawData[ImgHeight * ImgWidth + 2 + 4]  << 16);

    if ((0x01 << 3) & status)
    {
        return (1);
    }
    else
    {
        return (0);
    }
}

U08 GetLeptonGainMode(U16 *RawData)
{
    return (U08)RawData[ImgHeight * ImgWidth + 82 * 2 + 2 + 5];
}

/////return the FccFpaTemp
F32 GetFccFpaTemp(U16 *RawData)
{
    F32 ret;
    ret = (F32) RawData[ImgHeight * ImgWidth + 2 + 29];
    ret = ret / 100.0;
    return ret;
}
S32 GetTResoution(U16 *RawData)
{
    return RawData[ImgHeight * ImgWidth + 82 * 2 + 2 + 49];
}



//////////
F32 AccuracyRaw2Temp(S32 RawValue)
{
    //  static F32 lastTemp = 0.0f;
    F32 ret = 0.0f;

    if (GainMode == GAIN_MODE_HIGH) //high gain mode for lepton2.0,2.5,3.0,3.5
    {
        if (RawValue < g_GetRawData[1])
        {
            ret = RawValue * g_CalibrateCoef_A[0] + g_CalibrateCoef_B[0];
        }
        else if ((RawValue >= g_GetRawData[1]) && (RawValue < g_GetRawData[2]))
        {
            ret = RawValue * g_CalibrateCoef_A[1] + g_CalibrateCoef_B[1];
        }
        else if ((RawValue >= g_GetRawData[2]) && (RawValue < g_GetRawData[3]))
        {
            ret = RawValue * g_CalibrateCoef_A[2] + g_CalibrateCoef_B[2];
        }
        else if ((RawValue >= g_GetRawData[3]))
        {
            ret = RawValue * g_CalibrateCoef_A[3] + g_CalibrateCoef_B[3];
        }
    }
    else if (GainMode == GAIN_MODE_LOW) //low gain mode for lepton2.5,3.5
    {
        if (RawValue < g_GetRawData[6])
        {
            ret = RawValue * g_CalibrateCoef_A[5] + g_CalibrateCoef_B[5];
        }
        else if ((RawValue >= g_GetRawData[6]) && (RawValue < g_GetRawData[7]))
        {
            ret = RawValue * g_CalibrateCoef_A[6] + g_CalibrateCoef_B[6];
        }
        else if ((RawValue >= g_GetRawData[7]) && (RawValue < g_GetRawData[8]))
        {
            ret = RawValue * g_CalibrateCoef_A[7] + g_CalibrateCoef_B[7];
        }
        else if ((RawValue >= g_GetRawData[8]) && (RawValue < g_GetRawData[9]))
        {
            ret = RawValue * g_CalibrateCoef_A[8] + g_CalibrateCoef_B[8];
        }
        else if ((RawValue >= g_GetRawData[9]) && (RawValue < g_GetRawData[10]))
        {
            ret = RawValue * g_CalibrateCoef_A[9] + g_CalibrateCoef_B[9];
        }
        else if ((RawValue >= g_GetRawData[10]) && (RawValue < g_GetRawData[11]))
        {
            ret = RawValue * g_CalibrateCoef_A[10] + g_CalibrateCoef_B[10];
        }
        else if ((RawValue >= g_GetRawData[11]))
        {
            ret = RawValue * g_CalibrateCoef_A[11] + g_CalibrateCoef_B[11];
        }
    }

    /*   if (fabs(ret - 0.0f) <= 0.001f)
      {
        ret = lastTemp;
      }
      else
      {
        lastTemp = ret;
      }
     */
    return ret;
}


/* S32 AccuracyTemp2Raw(F32 Temp)
{
  F32 Ktemp;
  S32 RawData;
  Ktemp = Temp + 273.15;
  if(Ktemp <= g_TargetTemp[1])
    RawData = (S32)((Ktemp - g_CalibrateCoef_B[0])/g_CalibrateCoef_A[0]);
  else if(Ktemp > g_TargetTemp[3])
    RawData = (S32)((Ktemp - g_CalibrateCoef_B[3])/g_CalibrateCoef_A[3]);
  else if( (Ktemp > g_TargetTemp[1]) && (Ktemp <= g_TargetTemp[2]) )
    RawData = (S32)((Ktemp - g_CalibrateCoef_B[1])/g_CalibrateCoef_A[1]);
  else if( (Ktemp > g_TargetTemp[2]) && (Ktemp <= g_TargetTemp[3]) )
    RawData = (S32)((Ktemp - g_CalibrateCoef_B[2])/g_CalibrateCoef_A[2]);

  return RawData;
} */



//void ImgSideEnhance(S32 OrgData[][ImgWidth], S32 IgnoreSize, S32 Jvalue, F32 Gain)
//{
//    int i, j;
//    int TempSum;
//    for (i = 0; i < ImgHeight; i++)
//        for (j = 0; j < ImgWidth; j++)
//        {
//            if ((i == 0) || (i == ImgHeight - 1) || (j == 0) || (j == ImgWidth - 1))
//                TempArray[i][j] = 0;
//            else
//                TempArray[i][j] = OrgData[i - 1][j - 1] * TempSideEn[0][0] + OrgData[i - 1][j] * TempSideEn[0][1] + OrgData[i - 1][j + 1] * TempSideEn[0][2] +
//                OrgData[i - 0][j - 1] * TempSideEn[1][0] + OrgData[i - 0][j - 0] * TempSideEn[1][1] + OrgData[i - 0][j + 1] * TempSideEn[1][2] +
//                OrgData[i + 1][j - 1] * TempSideEn[2][0] + OrgData[i + 1][j - 0] * TempSideEn[2][1] + OrgData[i + 1][j + 1] * TempSideEn[2][2];
//
//        }
//
//#if 1
//        for (i = IgnoreSize / 2; i < ImgHeight - IgnoreSize / 2; i++)
//            for (j = IgnoreSize / 2; j < ImgWidth - IgnoreSize / 2; j++)
//            {
//                if (abs(TempArray[i][j]) < Jvalue)
//                {
//                    TempArray[i][j] = 0;
//                }
//                else
//                {
//                    TempSum = 0;
//                    for (int k = -IgnoreSize / 2; k <= IgnoreSize / 2; k++)
//                        for (int m = -IgnoreSize / 2; m <= IgnoreSize / 2; m++)
//                            if (abs(TempArray[i + k][j + m]) > Jvalue)
//                                TempSum += 1;
//
//                    if (TempSum < 2)
//                        TempArray[i][j] = 0;
//                    else if (abs(TempArray[i][j]) > 0)
//                        TempArray[i][j] -= Jvalue;
//                    else
//                        TempArray[i][j] += Jvalue;
//
//                }
//            }
//#endif // 0
//
//
//            for (i = 0; i < ImgHeight; i++)
//                for (j = 0; j < ImgWidth; j++)
//                {
//                    OrgData[i][j] = 0 + OrgData[i][j] + (S32)(Gain * TempArray[i][j]); //
//                    if (OrgData[i][j] < 0)
//                        OrgData[i][j] = 0;
//                    else if (OrgData[i][j] > 255)
//                        OrgData[i][j] = 255;
//                }
//
//}


void HistTableCreate(S32 OrgImg[][ImgWidth], S32 DivSize, F32 Gvalue, F32 Gain) {
    S32 i, j, k, m, n, l;
    F32 AvgVal, DifVal, SubAvgVal, SubDifVal;
    S32 WdirSubAreaNum;
    S32 HdirSubAreaNum;


    AvgVal = 0;
    DifVal = 0;
    for (i = 0; i < HistNum; i++) {
        HistTable[i] = 0;
        HistTabTemp[i] = 0;
    }

    for (i = 0; i < ImgHeight; i++)
        for (j = 0; j < ImgWidth; j++)
            AvgVal += OrgImg[i][j];

    AvgVal = AvgVal / ImgHeight / ImgWidth;

    for (i = 0; i < ImgHeight; i++)
        for (j = 0; j < ImgWidth; j++)
            DifVal += fabs(OrgImg[i][j] - AvgVal);

    DifVal = DifVal / ImgHeight / ImgWidth;

    WdirSubAreaNum = ImgWidth / DivSize;
    HdirSubAreaNum = ImgHeight / DivSize;
    for (i = 0; i < HdirSubAreaNum; i++)
        for (j = 0; j < WdirSubAreaNum; j++) {
            SubAvgVal = 0;
            SubDifVal = 0;
            m = i * DivSize;
            n = j * DivSize;

            for (k = m; k < m + DivSize; k++)
                for (l = n; l < n + DivSize; l++)
                    SubAvgVal = SubAvgVal + OrgImg[k][l];
            SubAvgVal = SubAvgVal / DivSize / DivSize;

            for (k = m; k < m + DivSize; k++)
                for (l = n; l < n + DivSize; l++)
                    SubDifVal += fabs(SubAvgVal - OrgImg[k][l]);
            SubDifVal = SubDifVal / DivSize / DivSize;

            if (SubDifVal / DifVal > Gvalue) {
                for (k = m; k < m + DivSize; k++)
                    for (l = n; l < n + DivSize; l++)
                        HistTabTemp[OrgImg[k][l]] += Gain * SubDifVal / DifVal;
            }

        }

    for (i = 0; i < HistNum; i++) {
        HistTable[i] = (S32) HistTabTemp[i];
    }

}

//MidValue Image avg
//HighRate/LowRate：calculate Image scale
//LowCutValue/HighCutValue:calculate plateau
//UnzAddValue/AddValue:
void ImgCreate(S32 OrgImg[][ImgWidth], S32 MidValue, F32 HighRate, F32 LowRate, S32 LowCutValue, S32 HighCutValue, S32 UnzAddValue, S32 AddValue)
{
    S32 i, j, k;
    S32 TotalNum = 0;
    S32 HighNum, LowNum, ImgGrayRange;
    S32 MaxData = 0;
    S32 MinData = 0;
    S32 Temp = 0;

    for(i = 0; i < HistNum; i++)
    {
        TotalNum += HistTable[i];
    }
    HighNum = (S32)((F32)TotalNum * HighRate);
    LowNum = (S32)((F32)TotalNum * LowRate);

    for(i = 0; i < HistNum; i++)
    {
        Temp += HistTable[i];
        if(Temp >= LowNum)
        {
            MinData = i;
            break;
        }
    }
    Temp = 0;
    for(i = HistNum - 1; i > 0; i--)
    {
        Temp += HistTable[i];
        if(Temp >= HighNum)
        {
            MaxData = i;
            break;
        }
    }

    if(MaxData - MinData >= 150)
        ImgGrayRange = 255;
    else
        ImgGrayRange = 255 * (MaxData - MinData) / (300 - MaxData + MinData);

    for(i = 1; i < HistNum; i++)
    {
        if(HistTable[i] == 0)
            HistTable[i] += AddValue;
        else if(HistTable[i] > HighCutValue)
            HistTable[i] = HighCutValue + UnzAddValue;
        else if(HistTable[i] < LowCutValue)
            HistTable[i] = LowCutValue + UnzAddValue;
        else
            HistTable[i] += UnzAddValue;

        HistTable[i] += HistTable[i - 1];
        //HistTable[i] +=
    }

    Temp = 0;
    for(i = 0; i < ImgHeight; i++)
        for(j = 0; j < ImgWidth; j++)
        {
            OrgImg[i][j] = ImgGrayRange * HistTable[OrgImg[i][j]] / HistTable[HistNum - 1];
            Temp += OrgImg[i][j];
        }
        Temp = Temp / (ImgHeight * ImgWidth);
        for(i = 0; i < ImgHeight; i++)
            for(j = 0; j < ImgWidth; j++)
            {
                OrgImg[i][j] = OrgImg[i][j] + MidValue - Temp;
                if(OrgImg[i][j] < 0)
                    OrgImg[i][j] = 0;
                else if(OrgImg[i][j] > 255)
                    OrgImg[i][j] = 255;
            }


}



S32 AcuImgCalc(F32 ImgArea)
{
    S32 CurFccTime;
    S32 FpaRaw;
    S32 Index;
    F32 CalcTemp;
    F32 HighLimit;
    F32 LowLimit;
    S32 FilterTemp;
    S32 RawDataTemp;
    int i = 0, j = 0;
    F32 MaxTemp, MinTemp;
    static U08 manualFcc = 0;

    if (IR_PLUG_SDK_ERR_SUCCESS != IR_PLUG_SDK_ReadOriginFrame(RawData, sizeof(RawData)))
    {
        return (-1);
    }

    RawDataConvert(RawData);

    g_LeptonCalibrating = GetLeptonCalibrationStatus(RawData);
    CurFccTime = GetLeptonFccTime(RawData);
    FpaRaw = DefaultTemp2Raw(GetFccFpaTemp(RawData));

    g_BootTime = GetLeptonBootTime(RawData);
    g_Fcctime = CurFccTime;


    // 开机 50 ~ 55 秒间没有校准，则手动校准一次，确保开机 60 秒后温度稳定
    if (0 == manualFcc && g_BootTime < 60 && g_BootTime >= 55 && g_Fcctime > 5)
    {
        // 发送 FCC 校准命令
        // 待实现


        manualFcc = 1;
    }

    if (g_BootTime >= 60 && g_Fcctime >= 5)
    {
        // 开机大于 60 秒后，lepton 自动校准 5 秒后认为温度稳定
        g_TempValueStable = 1;
    }
    else
    {
        g_TempValueStable = 0;
    }

    if (CurFccTime > FccPeriod)
    {
        CurFccTime = FccPeriod;
    }
    else if (CurFccTime < 0)
    {
        CurFccTime = 0;
    }

#if 1

    for (i = 0; i < ImgHeight; i++)         //gausfilter
    {
        Index = i * ImgWidth;

        for (j = 0; j < ImgWidth; j++)
        {
            RawBufForGausfillter[i][j] = RawData[Index + j];
        }
    }

    for (i = 1; i < ImgHeight - 1; i++)
    {
        Index = i * ImgWidth;

        for (j = 1; j < ImgWidth - 1; j++)
        {
#if 1
            FilterTemp = RawBufForGausfillter[i - 1][j - 1] * AcuGausTemp[0][0]
                         + RawBufForGausfillter[i - 1][j] * AcuGausTemp[0][1]
                         + RawBufForGausfillter[i - 1][j + 1] * AcuGausTemp[0][2]
                         + RawBufForGausfillter[i][j - 1] * AcuGausTemp[1][0]
                         + RawBufForGausfillter[i][j] * AcuGausTemp[1][1]
                         + RawBufForGausfillter[i][j + 1] * AcuGausTemp[1][2]
                         + RawBufForGausfillter[i + 1][j - 1] * AcuGausTemp[2][0]
                         + RawBufForGausfillter[i + 1][j] * AcuGausTemp[2][1]
                         + RawBufForGausfillter[i + 1][j + 1] * AcuGausTemp[2][2];
            RawData[Index + j] = FilterTemp / 18;
#endif

#if 0
            FilterTemp = RawBufForGausfillter[i - 1][j - 1] * GausTemp[0][0]
                         + RawBufForGausfillter[i - 1][j] * GausTemp[0][1]
                         + RawBufForGausfillter[i - 1][j + 1] * GausTemp[0][2]
                         + RawBufForGausfillter[i][j - 1] * GausTemp[1][0]
                         + RawBufForGausfillter[i][j] * GausTemp[1][1]
                         + RawBufForGausfillter[i][j + 1] * GausTemp[1][2]
                         + RawBufForGausfillter[i + 1][j - 1] * GausTemp[2][0]
                         + RawBufForGausfillter[i + 1][j] * GausTemp[2][1]
                         + RawBufForGausfillter[i + 1][j + 1] * GausTemp[2][2];
            RawData[Index + j] = FilterTemp / 10000;
#endif
        }
    }

#endif

    for (i = 0; i < ImgHeight; i++)
    {
        for (j = 0; j < ImgWidth; j++)
        {

            Index = i * ImgWidth + j;
            RawDataTemp = (S32) RawData[Index];
            RawDataTemp = RawDataTemp - UniformComArray[CurFccTime][i][j];   //uniform compensate
            RawDataTemp = RawDataTemp - FccComArray[CurFccTime];             //Fcc compensate
            CalcTemp = AccuracyRaw2Temp(RawDataTemp);
            AcuImgSaveBuf[i][j] = CalcTemp / 20.0 - 273.15;

            if (CalcTemp - 273.15 * 20 < IMGLTEMP)
            {
                AcuImgBuf[i][j] = 0;
            }
            else if (CalcTemp - 273.15 * 20 > IMGHTEMP)
            {
                AcuImgBuf[i][j] = IMGHTEMP - IMGLTEMP;
            }
            else
            {
                AcuImgBuf[i][j] = (S32)(CalcTemp - 273.15 * 20 - IMGLTEMP);
            }
        }
    }

    HistTableCreate(AcuImgBuf, 40, 0.1f, 1.2f);
    ImgCreate(AcuImgBuf, 115, 0.05, 0.05, 5, 65, 0, 0);
//    ImgSideEnhance(AcuImgBuf, 6, 3, 0.1);


    return 0;
}



S32 RawDataConvert(U16 *RawData)
{
    S32 FpaRaw, TResolution;
    S32 TempRaw;
    TResolution = GetTResoution(RawData);
    int i = 0;

    if (LEPTON_2_0 == LeptonId || LEPTON_3_0 == LeptonId || LEPTON_UNKNOW == LeptonId)
    {
        GainMode = GAIN_MODE_HIGH;

        //lepton 2.0/3.0 process
        FpaRaw = DefaultTemp2Raw(GetFccFpaTemp(RawData));

        for (i = 0; i < ImgHeight * ImgWidth; i++)
        {
            {
                TempRaw = (S32)(((F32) RawData[i] - g_IrWindowRe * (F32) FpaRaw)
                                / (1.0 - g_IrWindowRe));
                RawData[i] = (S16)(DefaultRaw2Temp(TempRaw) * 20.0);
            }
        }
    }
    else if (LEPTON_2_5 == LeptonId || LEPTON_3_5 == LeptonId)
    {
        //lepton 2.5/3.5 process
        if (0 == TResolution)
        {
            GainMode = GAIN_MODE_LOW;

            //lepton 2.5/3.5 low gain mode process
            for (i = 0; i < ImgHeight * ImgWidth; i++)
            {
                RawData[i] = RawData[i] * 2;
            }
        }
        else
        {
            GainMode = GAIN_MODE_HIGH;

            //lepton 2.5/3.5 high gain mode process
            for (i = 0; i < ImgHeight * ImgWidth; i++)
            {
                RawData[i] = RawData[i] / 5;
            }
        }
    }


    return 0;
}




S32 DefaultImgCalc()
{
    S32 MinValue, MaxValue, i, j, Index;

    MinValue = 80000;
    MaxValue = 0;

    IR_PLUG_SDK_ReadOriginFrame(RawData, sizeof(RawData));

    RawDataConvert(RawData);

    for (i = 0; i < ImgHeight * ImgWidth; i++)
    {
        if (MinValue > RawData[i])
        {
            MinValue = RawData[i];
        }

        if (MaxValue < RawData[i])
        {
            MaxValue = RawData[i];
        }
    }

    for (i = 0; i < ImgHeight; i++)
    {
        for (j = 0; j < ImgWidth; j++)
        {
            Index = i * ImgWidth + j;
            DefaultImgBuf[i][j] = (RawData[Index] - MinValue) * 255 / (MaxValue - MinValue);
            AcuImgBuf[i][j] = DefaultImgBuf[i][j];
        }
    }

    return 0;
}



void ImageEnhance(S32 ImgSrc[][ImgWidth])
{
    S32 i, j, FilterTemp;
    memset(ImgHisTable, 0, sizeof(S32) * 256);
    memset(ImgSumHisTable, 0, sizeof(S32) * 256);

    for (i = 0; i < ImgHeight; i++)
    {
        for (j = 0; j < ImgWidth; j++)
        {
            ImgHisTable[ImgSrc[i][j]]++;
        }
    }


    for (i = 2; i < 256; i++)
    {
        if (ImgHisTable[i] > 0)
        {
            ImgHisTable[i] = ImgHisTable[i] + (S32)(800.0f * g_Gain);
        }
        else
        {
            ImgHisTable[i] = ImgHisTable[i] + (S32)(800.0f * g_Gain);
        }

        ImgSumHisTable[i] = ImgSumHisTable[i - 1] + ImgHisTable[i];
    }

    for (i = 0; i < ImgHeight; i++)
    {
        for (j = 0; j < ImgWidth; j++)
        {
            ImgSrc[i][j] = (U08)(255 * ImgSumHisTable[ImgSrc[i][j]] / ImgSumHisTable[255]);
        }
    }


    if (g_FilterSelected == 1)
    {
        for (i = 1; i < ImgHeight - 1; i++)
        {
            for (j = 1; j < ImgWidth - 1; j++)
            {
                FilterTemp = ImgSrc[i - 1][j - 1] * AdjGausTemp[0][0] +
                             ImgSrc[i - 1][j] * AdjGausTemp[0][1]
                             + ImgSrc[i - 1][j + 1] * AdjGausTemp[0][2]
                             + ImgSrc[i][j - 1] * AdjGausTemp[1][0]
                             + ImgSrc[i][j] * AdjGausTemp[1][1]
                             + ImgSrc[i][j + 1] * AdjGausTemp[1][2]
                             + ImgSrc[i + 1][j - 1] * AdjGausTemp[2][0]
                             + ImgSrc[i + 1][j] * AdjGausTemp[2][1]
                             + ImgSrc[i + 1][j + 1] * AdjGausTemp[2][2];

                if (FilterTemp < 0)
                {
                    FilterTemp = 0;
                }
                else if (FilterTemp > 255)
                {
                    FilterTemp = 255;
                }

                AcuImgTemp[i][j] = FilterTemp;
            }
        }

        for (i = 1; i < ImgHeight - 1; i++)
        {
            for (j = 1; j < ImgWidth - 1; j++)
            {
                ImgSrc[i][j] = AcuImgTemp[i][j];
            }
        }
    }
}



void ImageZoom(F32 *srcBuffer, U16 srcW, U16 srcH, F32 *dstBuffer, U08 scale)
{

    U32 i = 0, j = 0;
    F32 PixelTemp;
    F32 HeightCoef, WidthCoef;

    U32 index = 0;



    /**
     * 中间区域
     */
    for (i = 0; i < (srcH - 1) * scale; ++i)
    {
        for (j = 0; j < (srcW - 1) * scale; ++j)
        {
            HeightCoef = ((float) i) / ((float) scale) - i / scale;
            WidthCoef = ((float) j) / ((float) scale) - j / scale;

            index = (i / scale) * srcW + j / scale;

            PixelTemp = (float) srcBuffer[index] * (1.0f - HeightCoef) * (1.0f - WidthCoef) +
                        (float) srcBuffer[index + 1] * (1.0f - HeightCoef) * (WidthCoef) +
                        (float) srcBuffer[index + srcW] * (HeightCoef) * (1.0f - WidthCoef) +
                        (float) srcBuffer[index + srcW + 1] * (HeightCoef) * (WidthCoef);

            dstBuffer[i * srcW * scale + j] = (F32) PixelTemp;
        }
    }


    /**
     * 右侧出界处理
     */
    for (i = 0; i < (srcH - 1) * scale; ++i)
    {
        for (j = (srcW - 1) * scale; j < srcW * scale; ++j)
        {


            HeightCoef = ((float) i) / ((float) scale) - i / scale;
            WidthCoef = ((float) j) / ((float) scale) - j / scale;


            PixelTemp = (float) srcBuffer[index] * (1.0f - HeightCoef) * (1.0f - WidthCoef) +
                        (float)(2 * srcBuffer[index] - srcBuffer[index - 1])  * (1.0f - HeightCoef) * (WidthCoef) +
                        (float) srcBuffer[index + srcW] * (HeightCoef) * (1.0f - WidthCoef) +
                        (float)(2 * srcBuffer[index + srcW] - srcBuffer[index + srcW - 1]) * (HeightCoef) * (WidthCoef);

            dstBuffer[i * srcW * scale + j] = (F32) PixelTemp;
        }
    }



    /**
     * 下方出界处理
     */
    for (i = (srcH - 1) * scale; i < srcH * scale; ++i)
    {
        for (j = 0; j < (srcW - 1) * scale; ++j)
        {

            HeightCoef = ((float) i) / ((float) scale) - i / scale;
            WidthCoef = ((float) j) / ((float) scale) - j / scale;


            PixelTemp = (float) srcBuffer[index] * (1.0f - HeightCoef) * (1.0f - WidthCoef) +
                        (float) srcBuffer[index + 1] * (1.0f - HeightCoef) * (WidthCoef) +
                        (float)(2 * srcBuffer[index + srcW] - srcBuffer[index]) * (HeightCoef) * (1.0f - WidthCoef) +
                        (float)(2 * srcBuffer[index + srcW + 1] - srcBuffer[index + 1]) * (HeightCoef) * (WidthCoef);

            dstBuffer[i * srcW * scale + j] = (F32) PixelTemp;
        }
    }

    /**
     * 右下角出界
     */
    F32 value = srcBuffer[srcW * srcH - 1];

    for (i = (srcH - 1) * scale; i < srcH * scale; ++i)
    {
        for (j = (srcW - 1) * scale; j < srcW * scale; ++j)
        {
            dstBuffer[i * srcW * scale + j] = value;
        }
    }
}


void ImageRotate90(F32 *srcBuffer, F32 *dstBuffer)
{
    int pos = 0, dst = 0;

    int row, col;

    U16 height = 240;
    U16 width = 320;

    //  for (row = 0; row < height; ++row)
    //  {
    //    for (col = 0; col < width; ++col)
    //    {
    //      srcBuffer[row * width + col] = row;
    //    }
    //  }

    for (row = 0; row < height; ++row)
    {
        for (col = 0; col < width; ++col)
        {
            pos = row * (width) + col;
            dst = (col) * height + (height - row - 1);
            dstBuffer[dst] = srcBuffer[pos];
        }
    }
}

S32 AccuracyTemp2Raw(F32 Temp)
{
    return (0);
}







