#include "ir_platform.h"
#include "ir_plug_sdk_common.h"
#include "ir_plug_sdk_tac.h"
#include "ir_plug_sdk.h"
#include "ir_plug_sdk_glib.h"
#include "ir_plug_sdk_algorithm.h"
/*
	RawData 为在没校准前的温度值(Tc + 273.15)*20取整得到；
	Lepton2.0,Lepton3.0需要将传输的Raw数据补偿FPA反射后，通过DefaultRaw2Temp得到
	Lepton2.5,Lepton3.5由于传输上来的已经是温度值，所以可以进行简单的再转换
*/
static char *g_ModeInfo = "TAC";

static char *g_StateInfo[14] =
{
    "waiting...",
    "0",
    "35",
    "70",
    "120",
    "150",
    "Lg 0",
    "Lg 35",
    "Lg 70",
    "Lg 120",
    "Lg 150",
    "Lg 230",
    "Lg 300",
    "Lg 400"
};


S32 AcCalCalc()
{
    S32 i;
    F32 Ctemp0, Ctemp1;


    if(LEPTON_2_0 == LeptonId || LEPTON_3_0 == LeptonId)
    {
        for (i = 0; i < 4; i++)
        {
            Ctemp0 = (F32)g_GetRawData[i];
            Ctemp1 = (F32)g_GetRawData[i + 1];
            g_CalibrateCoef_A[i] = (g_TargetTemp[i + 1] - g_TargetTemp[i]) /
                                   (Ctemp1 - Ctemp0);
            g_CalibrateCoef_B[i] = g_TargetTemp[i + 1] - g_CalibrateCoef_A[i] * Ctemp1;
        }
        //    g_CalibrateCoef_A[4] = (F32)(DefaultTemp2Raw(g_TargetTemp[4]) - g_GetRawData[4]);
        //    g_CalibrateCoef_B[4] = (F32)(DefaultTemp2Raw(g_TargetTemp[0]) - g_GetRawData[0]);
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            Ctemp0 = (F32)g_GetRawData[i];
            Ctemp1 = (F32)g_GetRawData[i + 1];
            g_CalibrateCoef_A[i] = (g_TargetTemp[i + 1] - g_TargetTemp[i]) /
                                   (Ctemp1 - Ctemp0);
            g_CalibrateCoef_B[i] = g_TargetTemp[i + 1] - g_CalibrateCoef_A[i] * Ctemp1;
        }
        for (i = 5; i < 12; i++)
        {
            Ctemp0 = (F32)g_GetRawData[i];
            Ctemp1 = (F32)g_GetRawData[i + 1];
            g_CalibrateCoef_A[i] = (g_TargetTemp[i + 1] - g_TargetTemp[i]) /
                                   (Ctemp1 - Ctemp0);
            g_CalibrateCoef_B[i] = g_TargetTemp[i + 1] - g_CalibrateCoef_A[i] * Ctemp1;
        }
    }
    return 0;
}

////////set GetRawValue < MinValue to reset calculate state
U16 AcCalGrant(F32 GetTemp, F32 LowLimit, F32 HighLimit)
{
    if((GetTemp < LowLimit) || (GetTemp > HighLimit))
    {
        CalEnPoint = 0;
        return 2;  //not in allowed range
    }
    else if(CalEnPoint == 1)
    {
        CalEnPoint = 0;
        return 0;  //success
    }
    else
    {
        return 1;  //waitting for button clicked
    }
}



//Calibrate Order 25/36/75/30/0
S32 AccuracyCalibrate()
{
    S32 CurFccTime, CurBootTime;
    F32 FccFpaTemp;
    S32 Index, FilteredData;
    U16 ret = 3;

    S32 CalState = 0;
    F32 CalcTemp;
    U16 TempIndex = 0;


    F32 dispFcc = 0.0f;
    S32 dispFilter = 0.0f;

    while (IR_PLUG_SDK_MODE_TEMP_ACCURACY_CAL == g_SdkMode)
    {
        if (!g_WorkThreadRunning)
        {
            break;
        }

        DefaultImgCalc();      //Image process

        FccFpaTemp = GetFccFpaTemp(RawData);
        dispFcc = FccFpaTemp;
        CurBootTime = GetLeptonBootTime(RawData);
        CurFccTime = GetLeptonFccTime(RawData);

        //////////Need to add information(uniform calibrating)
        IR_ALG_ConvertMapRgb888((U32 *)g_FrameBufferRgb);

        /* 显示当前校准状态                                     */
        sprintf(g_StringBuffer, "%6s:%s", g_ModeInfo, g_StateInfo[CalState]);
        IR_PLUG_SDK_GLIB_DrawStringAtLine(0, (U08 *)g_StringBuffer);


        /* 十字光标             */
        IR_PLUG_SDK_GLIB_DrawCrosshair();



        Index = (ImgHeight + 1) * ImgWidth / 2;

        /////
#if 0
        FilteredData = RawData[Index - ImgWidth - 1] * GausTemp[0][0] + RawData[Index -
                       ImgWidth] * GausTemp[0][1] + RawData[Index - ImgWidth + 1] * GausTemp[0][2]
                       + RawData[Index - 1] * GausTemp[1][0] + RawData[Index] * GausTemp[1][1] +
                       RawData[Index + 1] * GausTemp[1][2]
                       + RawData[Index + ImgWidth - 1] * GausTemp[2][0] + RawData[Index + ImgWidth] *
                       GausTemp[2][1] + RawData[Index + ImgWidth + 1] * GausTemp[2][2];
        FilteredData = FilteredData / 10000;
#endif
#if 0
        FilteredData = RawData[Index - ImgWidth - 1] * AcuGausTemp[0][0] + RawData[Index -
                       ImgWidth] * AcuGausTemp[0][1] + RawData[Index - ImgWidth + 1] * AcuGausTemp[0][2]
                       + RawData[Index - 1] * AcuGausTemp[1][0] + RawData[Index] * AcuGausTemp[1][1] +
                       RawData[Index + 1] * AcuGausTemp[1][2]
                       + RawData[Index + ImgWidth - 1] * AcuGausTemp[2][0] + RawData[Index + ImgWidth] *
                       AcuGausTemp[2][1] + RawData[Index + ImgWidth + 1] * AcuGausTemp[2][2];
        FilteredData = FilteredData / 18;
#endif
        FilteredData = RawData[Index - ImgWidth - 1] + RawData[Index - ImgWidth] + RawData[Index - ImgWidth + 1]
                       + RawData[Index - 1] + RawData[Index] + RawData[Index + 1]
                       + RawData[Index + ImgWidth - 1] + RawData[Index + ImgWidth] + RawData[Index + ImgWidth + 1];
        FilteredData = FilteredData / 9;

        //        CalcTemp = ((F32)FilteredData - g_IrWindowRe * (F32)(DefaultTemp2Raw(
        //                        FccFpaTemp))) / (1.0 - g_IrWindowRe);
        //       FilteredData = (S32)CalcTemp;
        /////
        dispFilter = FilteredData;

        /* 显示当前温度值                                       */
        //sprintf(g_StringBuffer, "%6s:%.2f", "TEMP", DefaultRaw2Temp(FilteredData) - 273.15);
        sprintf(g_StringBuffer, "%6s:%.2f", "TEMP", ((F32)FilteredData / 20.0) - 273.15);
        IR_PLUG_SDK_GLIB_DrawStringAtLine(1, (U08 *)g_StringBuffer);


        /* 校准调试打印信息                 */
        sprintf(g_StringBuffer, "%.2f, %d, %d", dispFcc, dispFilter, CurFccTime);
        IR_PLUG_SDK_GLIB_DrawStringAtLine(2, (U08 *)g_StringBuffer);


        if ((CurBootTime <= ACBWTIME) || (CurFccTime < 120))
        {
            ret = 3; //waitting for adjust
        }
        //else if(CalState < 6)
        else if( (CalState < 14 ) && (CalState > 0))
        {
            ret = AcCalGrant((F32)FilteredData,
                AcOffsetGrantLow[CalState - 1], AcOffsetGrantHigh[CalState - 1]);
        }

        //print inf
        if(ret == 3)
        {
            //waiting fcc
            IR_PLUG_SDK_GLIB_SetTextColor(0xFFFFFFFF);
        }
        else if(ret == 2)
        {
            //waitint in range
            IR_PLUG_SDK_GLIB_SetTextColor(0xFFFFFF00);
        }
        else if(ret == 1)
        {
            //grant enable
            IR_PLUG_SDK_GLIB_SetTextColor(0xFF00FF00);
        }
        else
        {
            //adjusted
        }

        if((CalState == 0) && (CurBootTime > ACBWTIME) && (CurFccTime >= 120))
        {
            CalState = 1;
        }
        else if((CurBootTime > ACBWTIME) && (CurFccTime >= 120) && (ret == 0))
        {
            g_GetRawData[CalState - 1] = FilteredData;
            CalState += 1;
        }



        if ((LEPTON_2_5 == LeptonId || LEPTON_3_5 == LeptonId) && 6 == CalState)
        {
            // 增益模式切换控制为 LowGain
            IR_PLUG_SDK_SendCommand(IR_PLUG_DEV_SET_GAIN_MODE, GAIN_MODE_LOW);
        }

        //////Lepton2.5,3.5
        if ((CalState == 14) && (LEPTON_2_5 == LeptonId || LEPTON_3_5 == LeptonId))
        {
            AcCalCalc(); //after get each calibrate point,then calculate the result of accuracy calibratte
            break;
        }
        else if((CalState == 6) && (LEPTON_2_0 == LeptonId || LEPTON_3_0 == LeptonId))
        {
            AcCalCalc();
            break;
        }
    }

    if ((LEPTON_2_0 == LeptonId || LEPTON_3_0 == LeptonId) && 6 == CalState)
    {
        return (0);
    }
    else if ((LEPTON_2_5 == LeptonId || LEPTON_3_5 == LeptonId) && 14 == CalState)
    {
        // 增益模式切换控制为 HighGain，供验证使用
        IR_PLUG_SDK_SendCommand(IR_PLUG_DEV_SET_GAIN_MODE, GAIN_MODE_HIGH);
        return (0);
    }
    else 
    {
        return (-1);
    }
}











