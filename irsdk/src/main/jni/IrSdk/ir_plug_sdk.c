#include "ir_plug_sdk.h"
#include "ir_plug_sdk_algorithm.h"
#include "ir_plug_sdk_common.h"
#include "ir_plug_sdk_tac.h"
#include "ir_plug_sdk_tav.h"
#include "ir_plug_sdk_tuc.h"
#include "ir_plug_sdk_tuv.h"
#include "ir_plug_sdk_glib.h"
#include "ir_plug_device.h"

#include <pthread.h>
#include <unistd.h>

#define IR_ORGIN_FRAME_WDITH_20         (60)
#define IR_ORGIN_FRAME_HEIGHT_20        (80)

#define IR_ORGIN_FRAME_WDITH_30         (120)
#define IR_ORGIN_FRAME_HEIGHT_30        (160)

static IR_PLUG_SDK_IRType g_SelectIrType;
static U16 g_FrameWidth = 0, g_FrameHeight = 0;
static U32 g_FrameRawDataByteSize = 0;
static U16 *g_OriginFrameBuffer = 0;
static U16 *g_Rgb565FrameBuffer = 0;
static F32 *g_TempFrameBuffer = 0;

static U16 g_FrameRawBufferTemp[160 * 120] = {0};

static char g_SdkWorkDir[256] = {0};
static char g_TacParamsFilePath[256] = {0};
static char g_TucParamsFilePath[256] = {0};
static char g_DeviceInfoFilePath[256] = {0};

static FILE *g_pFileTacParams = NULL;
static FILE *g_pFileTucParams = NULL;
static const char *g_TacParamsFileName = "tac.dat";
static const char *g_TucParamsFileName = "tuc.dat";
static const char *g_DeviceInfoFileName = "device.dat";

/*
 * SDK 版本号。 类型 . 大版本 . 小版本
 *    类型： 1 普通单模组
 *           A 多模组
 */
static const char *g_SdkVersion = "1.0.1";
static IR_PLUG_SDK_DeviceInfoTypeDef g_SdkDeviceInfo;


/* 返回值判断全局变量                                                               */
static IR_PLUG_SDK_ErrorTypeDef g_RetErr = IR_PLUG_SDK_ERR_SUCCESS;

static IR_PLUG_DEV_Interface g_IrPlugDevInterface =
{
    IR_PLUG_DEV_Init,
    IR_PLUG_DEV_Deinit,
    IR_PLUG_DEV_ControlRead,
    IR_PLUG_DEV_ControlWrite,
    IR_PLUG_DEV_BulkRead,
    IR_PLUG_DEV_BulkWrite,
    IR_PLUG_DEV_GetErrorText
};

static IR_ALGORITHM_HandleDef g_IrAlgorithmHandle =
{
    IR_ALG_Init,
    IR_ALG_ImageProcess,
    IR_ALG_TempCalc,
    IR_ALG_ConvertMapRgb888,
    IR_ALG_Colormap
};


//static char g_StringBuffer[1024] = {0};



/* Private functions -------------------------------------------------------------------*/
static int writeSdkDeviceInfo(void);
static int writeTacParams(void);
static int readTacParams(void);
static int clearTacParams(void);
static int writeTucParams(void);
static int readTucParams(void);
static int clearTucParams(void);
static void setDefaultTacParams(void);
static void setDefaultTucParams(void);

static void checkLeptonVersion(void);


static int createWorkThread(void);
static int destroyWorkThread(void);
void *workThreadFunc(void *param);


/* Exported variables ------------------------------------------------------------------*/
/* 工作模式全局变量                                                                 */
volatile U08 g_SdkMode = IR_PLUG_SDK_MODE_INVALID;


/* 是否显示调试信息             */
VU08 g_DebugEnable = 0;


/* Exported functions ------------------------------------------------------------------*/
IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_Init(IR_PLUG_SDK_IRType irType,
        const char *irName, int irDf, int busNum, int devNum)
{
    memset(g_StringBuffer, 0, sizeof(g_StringBuffer));
    memset(&g_SdkDeviceInfo, 0, sizeof(g_SdkDeviceInfo));

    switch (irType)
    {
        case IR_PLUG_TYPE_2_0:
        {
            g_FrameWidth = IR_ORGIN_FRAME_WDITH_20;
            g_FrameHeight = IR_ORGIN_FRAME_HEIGHT_20;
            g_IrAlgorithmHandle.Init(IR_ORGIN_FRAME_WDITH_20,
                                     IR_ORGIN_FRAME_HEIGHT_20);
            break;
        }

        case IR_PLUG_TYPE_3_0:
        {
            g_FrameWidth = IR_ORGIN_FRAME_WDITH_30;
            g_FrameHeight = IR_ORGIN_FRAME_HEIGHT_30;
            g_IrAlgorithmHandle.Init(IR_ORGIN_FRAME_WDITH_30,
                                     IR_ORGIN_FRAME_HEIGHT_30);
            break;
        }

        default:
        {
            return (IR_PLUG_SDK_ERR_WRONG_PARAM);
        }
    }

    g_IrAlgorithmHandle.Colormap((U08) IR_PLUG_SDK_COLORMAP_HOTMETAL);

    g_FrameRawDataByteSize = g_FrameWidth * g_FrameHeight * sizeof(U16);

    /* 初始化设备           */
    if (IR_PLUG_DEV_SUCCESS != g_IrPlugDevInterface.init(irName, irDf, busNum, devNum))
    {
        snprintf(g_StringBuffer, sizeof(g_StringBuffer), "%s",
                 g_IrPlugDevInterface.errorText());
        return (IR_PLUG_SDK_ERR_DEVICE_NOT_READY);
    }


    /* 检测 lepton 类型配置                               */
    checkLeptonVersion();

    /* TUC 校准参数                                     */
    if (0 != readTucParams())
    {
        setDefaultTucParams();
    }

    /* TAC 校准参数                                     */
    if (0 != readTacParams())
    {
        setDefaultTacParams();
    }

    if (0 != createWorkThread())
    {
        return (IR_PLUG_SDK_ERR_WRONG_PARAM);
    }

    IR_PLUG_SDK_GLIB_SetDrawBuffer((U32 *) g_FrameBufferRgb);

    g_SdkMode = IR_PLUG_SDK_MODE_NORMAL;

    strcpy(g_SdkDeviceInfo.sdkVersion, g_SdkVersion);

    g_Emissivity = 0.95f;

    return (IR_PLUG_SDK_ERR_SUCCESS);

#if 0
    memset(g_StringBuffer, 0, sizeof(g_StringBuffer));

    switch (irType)
    {
    case IR_PLUG_TYPE_2_0:
    {
        g_FrameWidth = IR_ORGIN_FRAME_WDITH_20;
        g_FrameHeight = IR_ORGIN_FRAME_HEIGHT_20;
        g_IrAlgorithmHandle.Init(IR_ORGIN_FRAME_WDITH_20,
                                 IR_ORGIN_FRAME_HEIGHT_20);
        break;
    }

    case IR_PLUG_TYPE_3_0:
    {
        g_FrameWidth = IR_ORGIN_FRAME_WDITH_30;
        g_FrameHeight = IR_ORGIN_FRAME_HEIGHT_30;
        g_IrAlgorithmHandle.Init(IR_ORGIN_FRAME_WDITH_30,
                                 IR_ORGIN_FRAME_HEIGHT_30);
        break;
    }

    default:
    {
        return (IR_PLUG_SDK_ERR_WRONG_PARAM);
    }
    }

    g_IrAlgorithmHandle.Colormap((U08) IR_PLUG_SDK_COLORMAP_RAINBOW);

    g_FrameRawDataByteSize = g_FrameWidth * g_FrameHeight * sizeof(U16);

    /* 初始化设备           */
    if (IR_PLUG_DEV_SUCCESS != g_IrPlugDevInterface.init(irName, irDf, busNum, devNum))
    {
        snprintf(g_StringBuffer, sizeof(g_StringBuffer), "%s",
                 g_IrPlugDevInterface.errorText());
        return (IR_PLUG_SDK_ERR_DEVICE_NOT_READY);
    }

    g_SdkMode = IR_PLUG_SDK_MODE_NORMAL;

    return (IR_PLUG_SDK_ERR_SUCCESS);
#endif
}

IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_Deinit(void)
{
    if (IR_PLUG_DEV_SUCCESS != g_IrPlugDevInterface.deinit())
    {
        return (IR_PLUG_SDK_ERR_DEVICE_NOT_READY);
    }

    destroyWorkThread();

    return (IR_PLUG_SDK_ERR_SUCCESS);
}

IR_PLUG_SDK_ErrorTypeDef Ir_PLUG_SDK_SetWorkDir(const char *dir)
{
    if (NULL == dir)
    {
        return (IR_PLUG_SDK_ERR_WRONG_PARAM);
    }

    sprintf(g_TacParamsFilePath,  "%s/%s", dir, g_TacParamsFileName);
    sprintf(g_TucParamsFilePath,  "%s/%s", dir, g_TucParamsFileName);
    sprintf(g_DeviceInfoFilePath, "%s/%s", dir, g_DeviceInfoFileName);

    return (IR_PLUG_SDK_ERR_SUCCESS);
}

IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_ReadOriginFrame(
    unsigned short *pFrameBuffer,
    int bufferSize)
{
    if (NULL == pFrameBuffer)
    {
        return (IR_PLUG_SDK_ERR_WRONG_PARAM);
    }

    /* 读取当前测温环境信息                     */
    //     if (IR_PLUG_DEV_SUCCESS != g_IrPlugDevInterface.controlWrite(
    //         IR_PLUG_DEV_READ_PARAMS, 0, 0, 0))
    //     {
    //         sprintf_s(g_StringBuffer, sizeof(g_StringBuffer),
    //             g_IrPlugDevInterface.errorText());
    //         return (IR_PLUG_SDK_ERR_DEVICE_NOT_READY);
    //     }
    //     if (IR_PLUG_DEV_SUCCESS != g_IrPlugDevInterface.bulkRead(
    //         (unsigned char*)&g_ExtralInfo, sizeof(ExtraInfo)))
    //     {
    //         sprintf_s(g_StringBuffer, sizeof(g_StringBuffer),
    //             g_IrPlugDevInterface.errorText());
    //         return (IR_PLUG_SDK_ERR_DEVICE_NOT_READY);
    //     }
    //    g_ExtralInfo.atmTemp= 0;
    //    g_ExtralInfo.bkgTemp= 2941;
    //    g_ExtralInfo.Distance= 600;
    //    g_ExtralInfo.fpaTemp= 29418;
    //    g_ExtralInfo.emissivity= 1.0000000;
    //    g_ExtralInfo.Erwin= 0.20000000;
    //    g_ExtralInfo.MeasureB= 1402.1000;
    //    g_ExtralInfo.MeasureF= 3.0000000;
    //    g_ExtralInfo.MeasureR= 334980.00;
    //    g_ExtralInfo.MeasureO= 387.89999;
    //    g_ExtralInfo.ImgB= 1262.1000;
    //    g_ExtralInfo.ImgF= 3.0000000;
    //    g_ExtralInfo.ImgO= 723.97998;
    //    g_ExtralInfo.ImgR= 183080.00;

    /* 发送读取数据请求                         */
    if (IR_PLUG_DEV_SUCCESS != g_IrPlugDevInterface.controlWrite(
                IR_PLUG_DEV_READ_FRAME, 0, 0, 0))
    {
        snprintf(g_StringBuffer, sizeof(g_StringBuffer), "%s",
                 g_IrPlugDevInterface.errorText());
        return (IR_PLUG_SDK_ERR_DEVICE_NOT_READY);
    }

    /* 通过批量传输读取一帧数据                 */
    if (IR_PLUG_DEV_SUCCESS != g_IrPlugDevInterface.bulkRead(
                (unsigned char *) pFrameBuffer, bufferSize))
    {
        snprintf(g_StringBuffer, sizeof(g_StringBuffer), "%s",
                 g_IrPlugDevInterface.errorText());
        return (IR_PLUG_SDK_ERR_DEVICE_NOT_READY);
    }

    return (IR_PLUG_SDK_ERR_SUCCESS);
}

IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_SendCommand(
    IR_PLUG_SDK_CommandTypeDef cmd,
    unsigned short value)
{

    /* 发送读取数据请求                         */
    if (IR_PLUG_DEV_SUCCESS != g_IrPlugDevInterface.controlWrite(
                cmd, value, NULL, 0))
    {
        snprintf(g_StringBuffer, sizeof(g_StringBuffer), "%s",
                 g_IrPlugDevInterface.errorText());
        return (IR_PLUG_SDK_ERR_DEVICE_NOT_READY);
    }

    return (IR_PLUG_SDK_ERR_WRONG_PARAM);
}

IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_SetPallete(IR_PLUG_SDK_ColormapTypeDef type)
{
    g_IrAlgorithmHandle.Colormap(type % IR_PLUG_SDK_COLORMAP_INVALID);

    return (IR_PLUG_SDK_ERR_SUCCESS);
}

IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_SetLeptonType(U08 type)
{
    if (SOURCE_READ_USB == g_SdkDeviceInfo.hardwareInfoSource)
    {
        return (IR_PLUG_SDK_ERR_SET_TYPE_INVALID);
    }
    else
    {
        if (LEPTON_2_0 == type || LEPTON_2_5 == type ||
            LEPTON_3_0 == type || LEPTON_3_5 == type)
        {
            g_SdkDeviceInfo.hardwareInfo.leptonType = type;

            LeptonId = type;

            writeSdkDeviceInfo();

            return (IR_PLUG_SDK_ERR_SUCCESS);
        }
        else
        {
            return (IR_PLUG_SDK_ERR_WRONG_PARAM);
        }
    }
}
IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_SetEmissivity(F32 emissivity)
{
    if (emissivity >= 0.10f && emissivity <= 1.0f) {
        g_Emissivity = emissivity;
        return (IR_PLUG_SDK_ERR_SUCCESS);
    } else {
        return (IR_PLUG_SDK_ERR_WRONG_PARAM);
    }
}

IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_SetFpaTemp(F32 fpaTemp, U32 enable)
{
    if (0 != enable)
    {
        // 手动设置环境温度
        g_FpaTempMode = FPA_TEMP_MODE_MANUAL;
        g_CurrentFpaTemp = fpaTemp;
    }
    else
    {
        // 环境温度从模组获取
        g_FpaTempMode = FPA_TEMP_MODE_RAW;
    }

    return (IR_PLUG_SDK_ERR_SUCCESS);

}

const char *IR_PLUG_SDK_GetErrorText(void)
{
    return (g_StringBuffer);
}







/* ------------------------------------------------------------------------------------------- */




///< 唤醒
IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_WakeUp()
{
    if (IR_PLUG_SDK_MODE_NORMAL == g_SdkMode)
    {
        return (IR_PLUG_SDK_ERR_SUCCESS);
    }

    g_RetErr = IR_PLUG_SDK_SendCommand(IR_PLUG_DEV_WAKE_UP, 0);

    if (IR_PLUG_SDK_ERR_SUCCESS == g_RetErr)
    {
        g_SdkMode = IR_PLUG_SDK_MODE_NORMAL;
    }

    return (g_RetErr);
}

///< 休眠
IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_Sleep()
{
    if (IR_PLUG_SDK_MODE_SLEEP == g_SdkMode)
    {
        return (IR_PLUG_SDK_ERR_SUCCESS);
    }

    g_RetErr = IR_PLUG_SDK_SendCommand(IR_PLUG_DEV_WAKE_UP, 0);

    if (IR_PLUG_SDK_ERR_SUCCESS == g_RetErr)
    {
        g_SdkMode = IR_PLUG_SDK_MODE_SLEEP;
    }

    return (g_RetErr);
}

///< 获取设备参数
IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_GetDeviceInfo(IR_PLUG_SDK_DeviceInfoTypeDef *pInfo)
{
    if (IR_PLUG_SDK_MODE_NORMAL != g_SdkMode)
    {
        return (IR_PLUG_SDK_ERR_DEVICE_NOT_READY);
    }

    memcpy(pInfo, &g_SdkDeviceInfo, sizeof(g_SdkDeviceInfo));

    return (IR_PLUG_SDK_ERR_SUCCESS);
}

///< Shutter校准
IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_ShutterCalibration()
{
    if (IR_PLUG_SDK_MODE_NORMAL != g_SdkMode)
    {
        return (IR_PLUG_SDK_ERR_DEVICE_NOT_READY);
    }

    return IR_PLUG_SDK_SendCommand(IR_PLUG_DEV_SHUTTER_CAL, 0);
}


///< 获取一副图像（返回RGB，返回处理状态）
IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_ReadImageRgb(U32 *imgBuffer, U32 buffSize)
{
    if (NULL == imgBuffer)
    {
        return (IR_PLUG_SDK_ERR_WRONG_PARAM);
    }

    if (g_FrameWidth * g_FrameHeight * sizeof(imgBuffer[0]) > buffSize)
    {
        return (IR_PLUG_SDK_ERR_WRONG_PARAM);
    }

    if (IR_PLUG_SDK_MODE_NORMAL == g_SdkMode)
    {
        AcuImgCalc(g_TempRange);
        IR_ALG_ConvertMapRgb888((U32 *) g_FrameBufferRgb);


        if (g_DebugEnable)
        {
            /* 显示当前校准状态                                     */
            sprintf(g_StringBuffer, "%d", AcuImgSaveBuf[60][80]);
            IR_PLUG_SDK_GLIB_DrawStringAtLine(0, (U08 *)g_StringBuffer);
        }
    }

    memcpy(imgBuffer, g_FrameBufferRgb,
           g_FrameWidth * g_FrameHeight * sizeof(imgBuffer[0]));


    return (IR_PLUG_SDK_ERR_SUCCESS);

#if 0

    if (NULL == imgBuffer)
    {
        return (IR_PLUG_SDK_ERR_WRONG_PARAM);
    }

    if (g_FrameWidth * g_FrameHeight * sizeof(imgBuffer[0]) > buffSize)
    {
        return (IR_PLUG_SDK_ERR_WRONG_PARAM);
    }


    if (IR_PLUG_SDK_ERR_SUCCESS != IR_PLUG_SDK_ReadOriginFrame(
                g_DataBufferRaw, g_FrameRawDataByteSize))
    {
        return (IR_PLUG_SDK_ERR_DEVICE_NOT_READY);
    }

    memcpy(g_FrameRawBufferTemp, g_DataBufferRaw, g_FrameRawDataByteSize);

    g_IrAlgorithmHandle.Process(g_DataBufferRaw);
    g_IrAlgorithmHandle.Rgb(imgBuffer);


    /* 针对不同的模式，在图像中添加对应的信息                                       */
    if (IR_PLUG_SDK_MODE_TEMP_ACCURACY_CAL == g_SdkMode)
    {
    }
    else if (IR_PLUG_SDK_MODE_TEMP_ACCURACY_VERIFY == g_SdkMode)
    {
    }
    else if (IR_PLUG_SDK_MODE_TEMP_UNIFORMITY_CAL == g_SdkMode)
    {
    }
    else if (IR_PLUG_SDK_MODE_TEMP_UNIFORMITY_VERIFY == g_SdkMode)
    {
    }


    return (IR_PLUG_SDK_ERR_SUCCESS);
#endif
}

IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_GetFrameRawBuffer(unsigned short **pFrameRawBuffer)
{
    //     memcpy(pFrameRawBuffer, g_FrameRawBufferTemp, g_FrameRawDataByteSize);

    *pFrameRawBuffer = g_FrameRawBufferTemp;

    return (IR_PLUG_SDK_ERR_SUCCESS);
}

///< 捕获一副图像（传入图像个数，在按图像保存后，返回一个温度数组，一幅RGB图像，返回状态）
IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_Capture(
    U32 *imgBuffer, U32 imgBuffSize,
    F32 *tempBuffer, U32 tempBuffSize)
{
    if (NULL == imgBuffer)
    {
        return (IR_PLUG_SDK_ERR_WRONG_PARAM);
    }

    if (g_FrameWidth * g_FrameHeight * sizeof(imgBuffer[0]) > imgBuffSize)
    {
        return (IR_PLUG_SDK_ERR_WRONG_PARAM);
    }

    if (g_FrameWidth * g_FrameHeight * sizeof(tempBuffer[0]) > tempBuffSize)
    {
        return (IR_PLUG_SDK_ERR_WRONG_PARAM);
    }

    if (IR_PLUG_SDK_MODE_NORMAL == g_SdkMode)
    {
        if (0 != AcuImgCalc(g_TempRange))
        {

            return (IR_PLUG_SDK_ERR_DEVICE_NOT_READY);
        }

        IR_ALG_ConvertMapRgb888((U32 *) g_FrameBufferRgb);

        if (g_DebugEnable)
        {
            /* 打印调试信息                                     */
            sprintf(g_StringBuffer, "%d,%d,%d",
                    AcuImgSaveBuf[60][80], g_BootTime, g_Fcctime);
            IR_PLUG_SDK_GLIB_DrawStringAtLine(0, (U08 *) g_StringBuffer);
            sprintf(g_StringBuffer, "%.2f", g_UniformData);
            IR_PLUG_SDK_GLIB_DrawStringAtLine(1, (U08 *) g_StringBuffer);
        }
    }


//    ImageZoom(&AcuImgSaveBuf[0][0], ImgWidth, ImgHeight, &g_FrameBufferTemp[0][0], 2);
//    ImageRotate90(&g_FrameBufferTemp[0][0], tempBuffer);


//     sprintf(g_StringBuffer, "%.2f", tempBuffer[160 * 320 + 120]);
//     IR_PLUG_SDK_GLIB_DrawStringAtLine(1, (U08 *)g_StringBuffer);

    memcpy(imgBuffer, g_FrameBufferRgb, g_FrameWidth * g_FrameHeight * sizeof(imgBuffer[0]));
    IR_ALG_ConvertTemperature(tempBuffer);

    return (IR_PLUG_SDK_ERR_SUCCESS);
}

///< 进入温度精度校准模式
IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_EnterTempAccuracyCalibrationMode()
{
    if (LEPTON_UNKNOW == LeptonId)
    {
        // 未设置 Lepton 类型，不能进行温度校准
        return (IR_PLUG_SDK_ERR_LEPT_TYPE_UNKNOW);
    }

    if ((SOURCE_READ_USB == g_SdkDeviceInfo.hardwareInfoSource) &&
        (LEPTON_2_5 == LeptonId || LEPTON_3_5 == LeptonId))
    {
        // 2.5，3.5校准，首先进入“高增益”模式
        IR_PLUG_SDK_SendCommand(IR_PLUG_DEV_SET_GAIN_MODE, GAIN_MODE_HIGH);
    }

    g_SdkMode = IR_PLUG_SDK_MODE_TEMP_ACCURACY_CAL;

    return (IR_PLUG_SDK_ERR_SUCCESS);
}
IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_AbortTempAccuracyCalibrationMode()
{
    g_SdkMode = IR_PLUG_SDK_MODE_NORMAL;

    return (IR_PLUG_SDK_ERR_SUCCESS);
}

///< 进入温度精准验证模式
IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_EnterTempAccuracyVerifyMode()
{

    g_SdkMode = IR_PLUG_SDK_MODE_TEMP_ACCURACY_VERIFY;

    return (IR_PLUG_SDK_ERR_SUCCESS);
}

///< 温度一致性校准模式
IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_EnterTempUniformityCalibrationMode()
{
    g_SdkMode = IR_PLUG_SDK_MODE_TEMP_UNIFORMITY_CAL;

    return (IR_PLUG_SDK_ERR_SUCCESS);
}
IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_AbortTempUniformityCalibrationMode()
{
    g_SdkMode = IR_PLUG_SDK_MODE_NORMAL;

    return (IR_PLUG_SDK_ERR_SUCCESS);
}

///< 温度一致性验证模式
IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_EnterTempUniformityVerifyMode()
{
    g_SdkMode = IR_PLUG_SDK_MODE_TEMP_UNIFORMITY_VERIFY;

    return (IR_PLUG_SDK_ERR_SUCCESS);
}

///< 获取设备状态
IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_GetDeviceState(U08 *state)
{

    *state = g_SdkMode;

    return (IR_PLUG_SDK_ERR_SUCCESS);
}


IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_StartTacCapture(void)
{
    CalEnPoint = 1;
    return (IR_PLUG_SDK_ERR_SUCCESS);
}


IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_EnableDebug(U08 enable)
{
    if (enable != 0)
    {
        g_DebugEnable = 1;
    }
    else
    {
        g_DebugEnable = 0;
    }

    return (IR_PLUG_SDK_ERR_SUCCESS);
}


IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_GetTemp(int x, int y, F32 *temp)
{

    U16 pos = y;
    y = 480 - x;
    x = pos;

    *temp = 0.0f;

    if (x > 0 && x < 640 &&
            y > 0 && y < 480)
    {
        *temp = (F32) AcuImgSaveBuf[y >> 2][x >> 2];
        return (IR_PLUG_SDK_ERR_SUCCESS);
    }

    return (IR_PLUG_SDK_ERR_WRONG_PARAM);
}

IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_SetFlashingPeriod(U08 onSec, U08 offSec)
{
    if (IR_PLUG_SDK_MODE_NORMAL != g_SdkMode)
    {
        return (IR_PLUG_SDK_ERR_DEVICE_NOT_READY);
    }

    return IR_PLUG_SDK_SendCommand(IR_PLUG_DEV_SET_LED_FLASHING_PERIOD, onSec << 8 | offSec);
}






















static int writeSdkDeviceInfo(void)
{
    FILE * pFileHandle = fopen(g_DeviceInfoFilePath, "wb+");

    if (NULL == pFileHandle)
    {
        return -1;
    }

    fwrite(&g_SdkDeviceInfo, 1, sizeof(g_SdkDeviceInfo), pFileHandle);

    fflush(pFileHandle);
    fclose(pFileHandle);

    return (0);
}



static int writeTacParams(void)
{
    g_pFileTacParams = fopen(g_TacParamsFilePath, "wb+");

    if (NULL == g_pFileTacParams)
    {
        return (-1);
    }

    fwrite(g_CalibrateCoef_A, 1, sizeof(g_CalibrateCoef_A), g_pFileTacParams);
    fwrite(g_CalibrateCoef_B, 1, sizeof(g_CalibrateCoef_B), g_pFileTacParams);
    fwrite(g_GetRawData, 1, sizeof(g_GetRawData), g_pFileTacParams);

    fflush(g_pFileTacParams);
    fclose(g_pFileTacParams);

    g_SdkDeviceInfo.tacParamValid = 1;

    time_t curTime;
    time(&curTime);
    struct tm * pTm = localtime(&curTime);
    sprintf(g_SdkDeviceInfo.tacDatetime, "%d-%02d-%02d %02d:%02d:%02d",
            pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday,
            pTm->tm_hour, pTm->tm_min, pTm->tm_sec);

    writeSdkDeviceInfo();

    return (0);
}
static int readTacParams(void)
{
    if (0 == g_SdkDeviceInfo.tacParamValid)
    {
        return (-1);
    }

    g_pFileTacParams = fopen(g_TacParamsFilePath, "rb");

    if (NULL == g_pFileTacParams)
    {
        return (-1);
    }

    fread(g_CalibrateCoef_A, 1, sizeof(g_CalibrateCoef_A), g_pFileTacParams);
    fread(g_CalibrateCoef_B, 1, sizeof(g_CalibrateCoef_B), g_pFileTacParams);
    fread(g_GetRawData, 1, sizeof(g_GetRawData), g_pFileTacParams);

    fclose(g_pFileTacParams);

    return (0);
}

static int clearTacParams(void)
{
    g_pFileTacParams = fopen(g_TacParamsFilePath, "wb+");

    if (NULL == g_pFileTacParams)
    {
        return (-1);
    }

    fclose(g_pFileTacParams);

    return (0);
}

static int writeTucParams(void)
{
    g_pFileTucParams = fopen(g_TucParamsFilePath, "wb+");

    if (NULL == g_pFileTucParams)
    {
        return (-1);
    }

    fwrite(UniformComArray, 1, sizeof(UniformComArray), g_pFileTucParams);
    fwrite(FccComArray, 1, sizeof(FccComArray), g_pFileTucParams);

    fflush(g_pFileTucParams);
    fclose(g_pFileTucParams);

    g_SdkDeviceInfo.tucParamValid = 1;

    time_t curTime;
    time(&curTime);
    struct tm * pTm = localtime(&curTime);
    sprintf(g_SdkDeviceInfo.tucDatetime, "%d-%02d-%02d %02d:%02d:%02d",
            pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday,
            pTm->tm_hour, pTm->tm_min, pTm->tm_sec);

    writeSdkDeviceInfo();

    return (0);
}

static int readTucParams(void)
{
    if (0 == g_SdkDeviceInfo.tucParamValid)
    {
        return (-1);
    }

    g_pFileTucParams = fopen(g_TucParamsFilePath, "rb");

    if (NULL == g_pFileTucParams)
    {
        return (-1);
    }

    fread(UniformComArray, 1, sizeof(UniformComArray), g_pFileTucParams);
    fread(FccComArray, 1, sizeof(FccComArray), g_pFileTucParams);

    fclose(g_pFileTucParams);

    return (0);
}

static int clearTucParams(void)
{
    g_pFileTucParams = fopen(g_TucParamsFilePath, "wb+");

    if (NULL == g_pFileTucParams)
    {
        return (-1);
    }

    fclose(g_pFileTucParams);

    return (0);
}


S32 DefaultGetRawData[] = {5450, 6150, 6850, 7850, 8450, 5450, 6150, 6850, 7850, 8450, 10050, 11450, 13450};
void setDefaultTacParams(void)
{

    int i = 0, j = 0, k = 0;

    /* TAC 默认参数                     */
    for (i = 0; i < 13; ++i)
    {
        g_CalibrateCoef_A[i] = 1.0f;
        g_CalibrateCoef_B[i] = 0.0f;
        g_GetRawData[i] = DefaultGetRawData[i];
    }
}
void setDefaultTucParams(void)
{
    /* TUC 默认参数                     */
    memset(UniformComArray, 0, sizeof(UniformComArray));
    memset(FccComArray, 0, sizeof(FccComArray));
}



void checkLeptonVersion(void)
{
    /* 通过 USB 读取设备硬件信息参数                               */
    PlugHwInfo hardwareInfo;
    U08 hardwareInfoValid = 0;
    if (IR_PLUG_DEV_SUCCESS != g_IrPlugDevInterface.controlRead(
        IR_PLUG_DEV_GET_HWINFO, 0, (U08*)&hardwareInfo, sizeof(hardwareInfo)))
    {
        hardwareInfoValid = 0;
        snprintf(g_StringBuffer, sizeof(g_StringBuffer), "%s",
            g_IrPlugDevInterface.errorText());
    }
    else
    {
        hardwareInfoValid = 1;
    }


    /* 通过本地存储的设备信息参数                                    */
    IR_PLUG_SDK_DeviceInfoTypeDef deviceInfo;
    U08 deviceInfoValid = 0;
    FILE * pFileHandle = fopen(g_DeviceInfoFilePath, "rb");

    if (NULL == pFileHandle)
    {
        deviceInfoValid = 0;
    }
    else
    {
        fread(&deviceInfo, 1, sizeof(deviceInfo), pFileHandle);
        fclose(pFileHandle);

        deviceInfoValid = 1;
    }



    if (1 == hardwareInfoValid)
    {
        if (1 == deviceInfoValid && 
            hardwareInfo.leptonType == deviceInfo.hardwareInfo.leptonType &&
            (0 == strcmp(hardwareInfo.leptonId, deviceInfo.hardwareInfo.leptonId)))
        {
            // 模组硬件没有任何变化，不做处理
        }
        else
        {
            // 清除当前 SDK 存储的数据
            memset(&deviceInfo, 0, sizeof(deviceInfo));

            // 拷贝当前 USB 模组硬件信息
            deviceInfo.hardwareInfoSource = SOURCE_READ_USB;
            memcpy(&deviceInfo.hardwareInfo, &hardwareInfo, sizeof(hardwareInfo));
        }
    }
    else
    {
        if (SOURCE_MANUAL_SET != deviceInfo.hardwareInfoSource)
        {
            deviceInfoValid = 0;
        }

        if (0 == deviceInfoValid)
        {
            /* 模组硬件数据无效， SDK 存储数据无效 */
            memset(&deviceInfo, 0, sizeof(deviceInfo));

            deviceInfo.hardwareInfoSource = SOURCE_MANUAL_SET;
            deviceInfo.hardwareInfo.leptonType = LEPTON_UNKNOW;
            sprintf(deviceInfo.hardwareInfo.leptonId, "FFFFFFFF");
        }
    }


    memcpy(&g_SdkDeviceInfo, &deviceInfo, sizeof(deviceInfo));

    LeptonId = g_SdkDeviceInfo.hardwareInfo.leptonType;

    writeSdkDeviceInfo();
}




volatile S08 g_WorkThreadRunning = 0;
pthread_t g_WorkThreadHandle = 0;

void *workThreadFunc(void *param)
{

    while (1)
    {

        if (!g_WorkThreadRunning)
        {
            break;
        }

        switch (g_SdkMode)
        {
        case IR_PLUG_SDK_MODE_TEMP_ACCURACY_CAL:
        {
            if (0 == AccuracyCalibrate())
            {
                g_SdkMode = IR_PLUG_SDK_MODE_TEMP_ACCURACY_VERIFY;
            }

            break;
        }

        case IR_PLUG_SDK_MODE_TEMP_ACCURACY_VERIFY:
        {
            if (0 == AccuracyVerify())
            {
                writeTacParams();
                g_SdkMode = IR_PLUG_SDK_MODE_NORMAL;
            }

            break;
        }

        case IR_PLUG_SDK_MODE_TEMP_UNIFORMITY_CAL:
        {
            if (0 == UniformCalibrate())
            {
                g_SdkMode = IR_PLUG_SDK_MODE_TEMP_UNIFORMITY_VERIFY;
            }

            break;
        }

        case IR_PLUG_SDK_MODE_TEMP_UNIFORMITY_VERIFY:
        {
            if (0 == UniformVerify())
            {
                writeTucParams();
                g_SdkMode = IR_PLUG_SDK_MODE_NORMAL;
            }

            break;
        }

        default:
        {
            //usleep(1000 * 100);
            break;
        }
        }
    }

    pthread_exit(NULL);

    return (0);
}

static int createWorkThread(void)
{
#if defined(WIN32)
    g_WorkThreadHandle = (HANDLE)_beginthreadex(NULL, 0, workThreadFunc, NULL, CREATE_SUSPENDED, NULL);

    if (NULL == g_WorkThreadHandle)
    {
        return (1);
    }

    g_WorkThreadRunning = 1;

    ResumeThread(g_WorkThreadHandle);
#else

    int res = 0;

    g_WorkThreadRunning = 0;

    /* 创建线程                                                             */
    res = pthread_create(&g_WorkThreadHandle, NULL, workThreadFunc, NULL);

    if (res != 0)
    {
        return (-1);
    }

    g_WorkThreadRunning = 1;

#endif

    return (0);
}

static int destroyWorkThread(void)
{
#if defined(WIN32)

    // 手动关闭线程句柄
    if (NULL != g_WorkThreadHandle)
    {
        g_WorkThreadRunning = 0;

        if (WAIT_OBJECT_0 == WaitForSingleObject(g_WorkThreadHandle, INFINITE))
        {
            CloseHandle(g_WorkThreadHandle);
            g_WorkThreadHandle = NULL;
        }
    }

#else

    if (0 != g_WorkThreadHandle)
    {

        /* 终止线程运行                                 */
        g_WorkThreadRunning = 0;

        /* 等待线程退出                                 */
        pthread_join(g_WorkThreadHandle, NULL);

        g_WorkThreadHandle = 0;
    }

#endif

    return (0);
}