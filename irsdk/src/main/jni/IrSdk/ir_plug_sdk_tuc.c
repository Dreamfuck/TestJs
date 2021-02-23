#include "ir_platform.h"
#include "ir_plug_sdk_common.h"
#include "ir_plug_sdk_tuc.h"
#include "ir_plug_sdk.h"
#include "ir_plug_sdk_glib.h"
#include "ir_plug_sdk_algorithm.h"



static char *g_ModeInfo = "TUC";
static char *g_StateInfo[6] =
{
    "0",
    "1",
    "2",
    "3",
    "4",
    "5"
};



//////calculate the uniform calibrate and store
/////Set FccTime 0 to reset the state
S32 TfCorrect(U16 *RawData, S32 FccTime, S32 FccNum)
{
    static S32 LastFccTime = 0;
    static S32 FrameCnt = 0;
    S32 i, j, Index;
    S32 result;

    if ((FccTime != LastFccTime) && (LastFccTime >= 2)
            && (FrameCnt > 0))        //different second and has passed more than 2s since last fcc
    {
        for (i = 0; i < ImgHeight * ImgWidth; i++) //Time average
        {
            RawBufForTfCorrect[i] /= FrameCnt;
        }

        for (i = 0; i < ImgHeight; i++)         //reshape
        {
            for (j = 0; j < ImgWidth; j++)
            {
                Index = i * ImgWidth + j;
                RawBufForGausfillter[i][j] = RawBufForTfCorrect[Index];
            }
        }

        for (i = 1; i < ImgHeight - 1; i++)         //gausfilter
        {
            for (j = 1; j < ImgWidth - 1; j++)
            {

#if 0
                RawBufForCal[i][j] =  RawBufForGausfillter[i - 1][j - 1] * GausTemp[0][0] +
                                      RawBufForGausfillter[i - 1][j] * GausTemp[0][1] + RawBufForGausfillter[i - 1][j
                                              + 1] * GausTemp[0][2]
                                      + RawBufForGausfillter[i][j - 1] * GausTemp[1][0] + RawBufForGausfillter[i][j] *
                                      GausTemp[1][1] + RawBufForGausfillter[i][j + 1] * GausTemp[1][2]
                                      + RawBufForGausfillter[i + 1][j - 1] * GausTemp[2][0] + RawBufForGausfillter[i +
                                              1][j] * GausTemp[2][1] + RawBufForGausfillter[i + 1][j + 1] * GausTemp[2][2];
                RawBufForCal[i][j] = RawBufForCal[i][j] / 10000;
#endif
                RawBufForCal[i][j] = RawBufForGausfillter[i - 1][j - 1] * AcuGausTemp[0][0] +
                                     RawBufForGausfillter[i - 1][j] * AcuGausTemp[0][1] + RawBufForGausfillter[i - 1][j
                                             + 1] * AcuGausTemp[0][2]
                                     + RawBufForGausfillter[i][j - 1] * AcuGausTemp[1][0] + RawBufForGausfillter[i][j] *
                                     AcuGausTemp[1][1] + RawBufForGausfillter[i][j + 1] * AcuGausTemp[1][2]
                                     + RawBufForGausfillter[i + 1][j - 1] * AcuGausTemp[2][0] + RawBufForGausfillter[i +
                                             1][j] * AcuGausTemp[2][1] + RawBufForGausfillter[i + 1][j + 1] * AcuGausTemp[2][2];
                RawBufForCal[i][j] = RawBufForCal[i][j] / 18;
            }
        }

        if (LastFccTime < 300)          //copy result to UniformComArray
        {
            if (FccNum == 0)
            {
                FccComArray[LastFccTime] = RawBufForCal[ImgHeight / 2][ImgWidth / 2];
            }
            else
            {
                FccComArray0[LastFccTime] = RawBufForCal[ImgHeight / 2][ImgWidth / 2];
            }

            for (i = 0; i < ImgHeight; i++)
            {
                for (j = 0; j < ImgWidth; j++)
                {
                    if (FccNum == 0)
                    {
                        UniformComArray[LastFccTime][i][j] = RawBufForCal[i][j] - RawBufForCal[ImgHeight
                                                             / 2][ImgWidth / 2];

                    }
                    else
                    {
                        UniformComArray0[LastFccTime][i][j] = RawBufForCal[i][j] -
                                                              RawBufForCal[ImgHeight / 2][ImgWidth / 2];
                    }

                }
            }
        }

        for (i = 0; i < ImgHeight * ImgWidth;
                i++)  //copy new data for time frame average
        {
            RawBufForTfCorrect[i] = (S32)(RawData[i]);
        }

        FrameCnt = 1;
        result = LastFccTime;
        LastFccTime = FccTime;
        return  result;
    }
    else if ((FccTime != LastFccTime) || (FccTime < 2))
    {
        for (i = 0; i < ImgHeight * ImgWidth; i++)
        {
            RawBufForTfCorrect[i] = (S32)RawData[i];
        }

        FrameCnt = 1;
        LastFccTime = FccTime;
        return  0;
    }
    else
    {
        for (i = 0; i < ImgHeight * ImgWidth; i++)
        {
            RawBufForTfCorrect[i] = RawBufForTfCorrect[i] + (S32)RawData[i];
        }

        FrameCnt = FrameCnt + 1;
        return  0;
    }

}




S32 UniformCalibrate()
{
    S32 CurFccTime, LastFccTime, CurBootTime;
    S32 CalState = 0;
    S32 ret, i, j, k;

    //CalState = 0        =>     Wait to first time calibrate
    //CalState = 1        =>     being the first time calibrate
    //CalState = 2        =>     being the second time calibrate
    //CalState = 3        =>     wait to the second time calibrate
    //CaliState = 4       =>     finish calibrate
    while (IR_PLUG_SDK_MODE_TEMP_UNIFORMITY_CAL == g_SdkMode)
    {
        if (!g_WorkThreadRunning)
        {
            break;
        }

        DefaultImgCalc();      //Image process
        CurBootTime = GetLeptonBootTime(RawData);
        CurFccTime =  GetLeptonFccTime(RawData);

        //////////Need to add information(uniform calibrating)
        IR_ALG_ConvertMapRgb888((U32 *)g_FrameBufferRgb);

        /* 显示当前校准状态                                     */
        sprintf(g_StringBuffer, "%-10s:%s", g_ModeInfo, g_StateInfo[CalState]);
        IR_PLUG_SDK_GLIB_DrawStringAtLine(0, (U08 *)g_StringBuffer);


        switch (CalState)
        {
        case 0:
        {

            if ((CurBootTime > 300) && (CurFccTime >= 2) && (CurFccTime <= 6))
            {
                CalState = 1;
            }
            else
            {
                CalState = 0;
            }

            LastFccTime = CurFccTime;
            break;
        }

        case 1:
        {

//            if(CurFccTime - LastFccTime > 1)
//                break;
            if ((CurFccTime < LastFccTime)
                    || (CurFccTime - LastFccTime > 1))        //unwanted change
            {
                CalState = 0;
                break;
            }
            else
            {
                ret = TfCorrect(RawData, CurFccTime, 0);
                LastFccTime = CurFccTime;

                if (ret == FccPeriod)
                {
                    CalState = 3;
                }
                else
                {
                    CalState = 1;
                }

                break;
            }
        }

        case 2:
        {
            if ((CurFccTime < LastFccTime)
                    || (CurFccTime - LastFccTime > 1))        //unwanted change
            {
                CalState = 3;
                break;
            }
            else
            {
                ret = TfCorrect(RawData, CurFccTime, 1);
                LastFccTime = CurFccTime;

                if (ret == FccPeriod)
                {
                    CalState = 4;
                }
                else
                {
                    CalState = 2;
                }

                break;
            }
        }

        case 3:
        {
            if ((CurBootTime > 600) && (CurFccTime >= 2) && (CurFccTime <= 6))
            {
                CalState = 2;
            }
            else
            {
                CalState = 3;
            }

            LastFccTime = CurFccTime;
            break;
        }

        case 4:
            break;

        default:
            break;
        }

        if (CalState == 4)
        {
            break;
        }
    }

    if (CalState == 4)
    {
        for (i = 0; i < 300; i++)
            for (j = 0; j < ImgHeight; j++)
                for (k = 0; k < ImgWidth; k++)
                {
                    UniformComArray[i][j][k] = (UniformComArray0[i][j][k]
                                                + UniformComArray[i][j][k]) / 2;
                }

        for (i = 0; i < 300; i++)
        {
            if (i > 120)
            {
                FccComArray[i] = 0;
            }
            else
            {
                FccComArray[i] = (FccComArray[i] - FccComArray[120]
                                  + FccComArray0[i] - FccComArray0[120]) / 2;
            }
        }

        return 0;
    }
    else
    {
        return(-1);
    }
}