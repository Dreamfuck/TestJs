#include "ir_platform.h"
#include "ir_plug_sdk_common.h"
#include "ir_plug_sdk_tav.h"
#include "ir_plug_sdk_tac.h"
#include "ir_plug_sdk.h"
#include "ir_plug_sdk_glib.h"
#include "ir_plug_sdk_algorithm.h"


#include <stdio.h>


static char *g_ModeInfo = "TAV";
//static char * g_StateInfo[6] = {
//    "waiting...",
//    "25",
//    "36",
//    "75",
//    "30",
//    "0"
//};
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


U16 AcVerifyGrant(F32 GetTemp, F32 MinValue, F32 MaxValue)
{
    if((GetTemp < MinValue) || (GetTemp > MaxValue))
    {
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





////////verify order 25/36/75/30/0
S32 AccuracyVerify()
{
    S32 CurFccTime, CurBootTime;
    S32 CalState = 0;
    F32 FccFpaTemp;
    F32 CalcTemp;
    S32 Index, FilteredData;
    U16 ret = 3;

    while (IR_PLUG_SDK_MODE_TEMP_ACCURACY_VERIFY == g_SdkMode)
    {
        if (!g_WorkThreadRunning)
        {
            break;
        }

        DefaultImgCalc();      //Image process
        FccFpaTemp = GetFccFpaTemp(RawData);
        CurBootTime = GetLeptonBootTime(RawData);
        CurFccTime = GetLeptonFccTime(RawData);
        Index = (ImgHeight + 1) * ImgWidth / 2;

        //////////Need to add information(uniform calibrating)
        IR_ALG_ConvertMapRgb888((U32 *)g_FrameBufferRgb);

        /* 显示当前验证状态                                     */
        sprintf(g_StringBuffer, "%6s:%s", g_ModeInfo, g_StateInfo[CalState]);
        IR_PLUG_SDK_GLIB_DrawStringAtLine(0, (U08 *)g_StringBuffer);


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

        /* 十字光标             */
        IR_PLUG_SDK_GLIB_DrawCrosshair();



        if (CurFccTime > FccPeriod)
        {
            FilteredData = FilteredData  - FccComArray[FccPeriod];
        }
        else
        {
            FilteredData = FilteredData - FccComArray[CurFccTime];
        }



        /* 显示当前温度值                                       */
        sprintf(g_StringBuffer, "%6s:%.2f", "TEMP", AccuracyRaw2Temp(FilteredData) / 20.0 - 273.15);
        IR_PLUG_SDK_GLIB_DrawStringAtLine(1, (U08 *)g_StringBuffer);
        /////

        if ((CurBootTime <= ACBWTIME) || (CurFccTime < 15))
        {
            ret = 3; //waitting for adjust
        }
        else if((CalState < 14) && (CalState > 0))
        {
            ret = AcVerifyGrant(AccuracyRaw2Temp(FilteredData),
                AvOffsetGrantLow[CalState - 1], AvOffsetGrantHigh[CalState - 1]);
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


        if((CalState == 0) && (CurBootTime > ACBWTIME) && (CurFccTime >= 15))
        {
            CalState = 1;
        }
        else if((CurBootTime > ACBWTIME) && (CurFccTime >= 15) && (ret == 0))
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
            break;
        }
        else if((CalState == 6) && (LEPTON_2_0 == LeptonId || LEPTON_3_0 == LeptonId))
        {
            break;
        }
    }


    if ((LEPTON_2_0 == LeptonId || LEPTON_3_0 == LeptonId) && 6 == CalState)
    {
        return (0);
    }
    else if ((LEPTON_2_5 == LeptonId || LEPTON_3_5 == LeptonId) && 14 == CalState)
    {
        // 增益模式切换控制为 AutoGain，供验应用程序使用
        IR_PLUG_SDK_SendCommand(IR_PLUG_DEV_SET_GAIN_MODE, GAIN_MODE_AUTO);
        return (0);
    }
    else
    {
        return (-1);
    }
}



