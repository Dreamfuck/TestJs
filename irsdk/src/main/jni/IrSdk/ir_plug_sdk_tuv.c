#include "ir_platform.h"
#include "ir_plug_sdk_common.h"
#include "ir_plug_sdk_tuv.h"
#include "ir_plug_sdk.h"
#include "ir_plug_sdk_glib.h"
#include "ir_plug_sdk_algorithm.h"

////////return 1 verify pass, 0 verify unpass
S32 UniformVerify()
{
    S32 CurFccTime, CurBootTime;
    S32 ret, i, j, Index;
    S32 MaxValue, MinValue;
    S32 ValidFrameCnt = 0;
    S32 UnValidFrameCnt = 0;
    S32 TempIndex;
    while (IR_PLUG_SDK_MODE_TEMP_UNIFORMITY_VERIFY == g_SdkMode)
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


        /* 显示当前验证状态							        */
        sprintf(g_StringBuffer, "TUV:%03d,%03d", ValidFrameCnt, UnValidFrameCnt);
        IR_PLUG_SDK_GLIB_DrawStringAtLine(0, (U08 *)g_StringBuffer);


        //        IR_PLUG_SDK_GLIB_DrawString(0, 0, (U08*)"TUV");




        MaxValue = 0;
        MinValue = 80000;

        if ((CurFccTime > 10) && (CurBootTime > 600) && (CurFccTime <= FccPeriod))
        {
            for (i = 0; i < ImgHeight; i++)         //reshape
            {
                for (j = 0; j < ImgWidth; j++)
                {
                    Index = i * ImgWidth + j;
                    RawBufForGausfillter[i][j] = (S32)RawData[Index];
                }
            }


            for (i = 1; i < ImgHeight - 1; i++)         //gausfilter and compensate
            {
                for (j = 1; j < ImgWidth - 1; j++)
                {
#if 0
                    RawBufForCal[i][j] = RawBufForGausfillter[i - 1][j - 1] * GausTemp[0][0]
                                         + RawBufForGausfillter[i - 1][j] * GausTemp[0][1]
                                         + RawBufForGausfillter[i - 1][j + 1] * GausTemp[0][2]
                                         + RawBufForGausfillter[i][j - 1] * GausTemp[1][0]
                                         + RawBufForGausfillter[i][j] * GausTemp[1][1]
                                         + RawBufForGausfillter[i][j + 1] * GausTemp[1][2]
                                         + RawBufForGausfillter[i + 1][j - 1] * GausTemp[2][0]
                                         + RawBufForGausfillter[i + 1][j] * GausTemp[2][1]
                                         + RawBufForGausfillter[i + 1][j + 1] * GausTemp[2][2];
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

                    RawBufForCal[i][j] = RawBufForCal[i][j] -
                                         UniformComArray[CurFccTime][i][j];
                }
            }

            for (i = 5; i < ImgHeight - 5; i++)
            {
                for (j = 5; j < ImgWidth - 5; j++)
                {
                    if (RawBufForCal[i][j] < MinValue)
                    {
                        MinValue = RawBufForCal[i][j];
                    }

                    if (RawBufForCal[i][j] > MaxValue)
                    {
                        MaxValue = RawBufForCal[i][j];
                    }
                }
            }
            TempIndex = UnValidFrameCnt + ValidFrameCnt;
            if (MaxValue - MinValue > 30)
            {
                UnValidFrameCnt++;
            }
            else
            {
                ValidFrameCnt++;
            }
        }


        if (UnValidFrameCnt + ValidFrameCnt > 900)
        {
            break;
        }

    }

    if (UnValidFrameCnt < 900)
    {
        ret = 0;
    }
    else
    {
        ret = -1;
    }

    return ret;
}