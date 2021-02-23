#pragma once

#include "ir_platform.h"

/**
  * @brief SDK 错误类型定义
  */
typedef enum
{
    IR_PLUG_SDK_ERR_SUCCESS = 0,            /*!< 操作成功           */
    IR_PLUG_SDK_ERR_WRONG_PARAM,            /*!< 函数参数错误       */
    IR_PLUG_SDK_ERR_SET_TYPE_INVALID,       /*!< 不支持手动设置 lepton 类型 */
    IR_PLUG_SDK_ERR_LEPT_TYPE_UNKNOW,       /*!< 不支持手动设置 lepton 类型 */
    IR_PLUG_SDK_ERR_DEVICE_NOT_READY,       /*!< IR 设备未准备就绪  */
}IR_PLUG_SDK_ErrorTypeDef;


/**
  * @brief SDK 工作模式定义
  */
typedef enum {
  IR_PLUG_SDK_MODE_NORMAL = 0,                /*!< 普通工作模式                                   */
  IR_PLUG_SDK_MODE_TEMP_ACCURACY_CAL,         /*!< 温度精度校准模式                               */
  IR_PLUG_SDK_MODE_TEMP_ACCURACY_VERIFY,      /*!< 温度精度验证模式                               */
  IR_PLUG_SDK_MODE_TEMP_UNIFORMITY_CAL,       /*!< 温度均匀性校准模式                             */
  IR_PLUG_SDK_MODE_TEMP_UNIFORMITY_VERIFY,    /*!< 温度均匀性验证模式                             */
  IR_PLUG_SDK_MODE_FIRMWARE_UPGRADE,          /*!< 固件升级模式                                   */
  IR_PLUG_SDK_MODE_SLEEP,                     /*!< 休眠模式                                       */


  IR_PLUG_SDK_MODE_INVALID
} IR_PLUG_SDK_ModeTypeDef;

/**
  * @brief IR 类别
  */
typedef enum
{
    IR_PLUG_TYPE_2_0        = 0x10,     /*!< 使用 2.0 版本 IR   */
    IR_PLUG_TYPE_3_0        = 0x20,     /*!< 使用 3.0 版本 IR   */
}IR_PLUG_SDK_IRType;



/**
  * @brief 配置操作支持的操作类别
  */
typedef enum
{
//     IR_PLUG_SDK_CMD_COLORMAP = 100,         /*!< 调色板                     */
//     IR_PLUG_SDK_CMD_EMISSIVITY,             /*!< 辐射率                     */
//     IR_PLUG_SDK_CMD_START_CALIBRATION,      /*!< 开始校准，进入校准模式     */
//     IR_PLUG_SDK_CMD_VERSION,                /*!< 获取版本信息               */


    IR_PLUG_DEV_READ_FRAME      = 0x10,             /*!< 请求读取一帧图像数据       */
    IR_PLUG_DEV_CMD,                         /*!< 设备配置请求               */
    IR_PLUG_DEV_READ_PARAMS,                        /*!< 获取当前测温环境参数       */
    IR_PLUG_DEV_WAKE_UP,                            /*!< 唤醒设备                   */
    IR_PLUG_DEV_SLEEP,                              /*!< 进入睡眠模式               */
    IR_PLUG_DEV_GET_HWINFO,                         /*!< 获取设备信息               */
    IR_PLUG_DEV_SET_GAIN_MODE,                      /*!< 设置 Lepton 的增益模式  */
    IR_PLUG_DEV_SHUTTER_CAL,                        /*!< 控制 shutter 校准          */
    IR_PLUG_DEV_SET_SHUTTER_CAL_PERIOD,             /*!< 设置 shutter 校准周期      */
    IR_PLUG_DEV_SET_LED_FLASHING_PERIOD,                      /*!< 设置 led 状态              */

}IR_PLUG_SDK_CommandTypeDef;


/**
  * @brief  调色板类型定义,用于命令 IR_PLUG_SDK_CMD_COLORMAP 参数
  */
typedef enum
{
    IR_PLUG_SDK_COLORMAP_HOTMETAL    = 0,                /*!<            */
    IR_PLUG_SDK_COLORMAP_GRAY,
    IR_PLUG_SDK_COLORMAP_RAINBOW,
    IR_PLUG_SDK_COLORMAP_HOTSPOT,
    IR_PLUG_SDK_COLORMAP_COLDSPOT,
    IR_PLUG_SDK_COLORMAP_INVALID
}IR_PLUG_SDK_ColormapTypeDef;




typedef enum enmLeptonType{
    LEPTON_2_0 = 20,
    LEPTON_2_5 = 25,
    LEPTON_3_0 = 30,
    LEPTON_3_5 = 35,
    LEPTON_UNKNOW = 0xFF,
}LeptonType;

typedef enum enmHardwareInfoSource{
    SOURCE_READ_USB  = 80,
    SOURCE_MANUAL_SET,
    SOURCE_INVALID
}HardwareInfoSource;

typedef struct tagPlugHwInfo
{
    int  leptonType;            // Lepton 型号
    char leptonId[20];          // Lepton 序列号
    char firmwareVersion[8];    // 固件版本号：1.0.0，2.0.1等
    char hardwareId[32];        // CPU ID
}PlugHwInfo;

/** 
  * @brief  设备信息类型定义
  */ 
typedef struct
{
    char sdkVersion[8];             /*!< 当前 SDK 版本信息，格式 x.x.x */

    int hardwareInfoSource;         /*!< 模组硬件信息来源 */
    PlugHwInfo hardwareInfo;        /*!< 模组硬件相关信息 */
    int  tacParamValid;             /*!< TAC 校准参数是否有效 */
    char tacDatetime[24];           /*!< TAC校准日期时间20170719 180101              */
    int  tucParamValid;             /*!< TUC 校准参数是否有效 */
    char tucDatetime[24];           /*!< TUC校准日期时间20170719 180101              */
}IR_PLUG_SDK_DeviceInfoTypeDef;




extern volatile S08 g_WorkThreadRunning;
extern volatile U08 g_SdkMode;

VU08 g_DebugEnable;



extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_Init(IR_PLUG_SDK_IRType irType, const char* irName, int irDf, int, int);
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_Deinit(void);
extern IR_PLUG_SDK_ErrorTypeDef Ir_PLUG_SDK_SetWorkDir(const char* dir);
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_ReadOriginFrame(unsigned short * pFrameBuffer, int bufferSize);
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_ReadRgb565Frame(unsigned short * pFrameBuffer, int bufferSize);
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_ReadTempFrame(double * pFrameBuffer, int bufferSize);
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_ConvertOrgin2Rgb565(unsigned short * pOriginBuffer, unsigned short * pRgb565Buffer);
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_ConvertOrgin2Temp(unsigned short * pOriginBuffer, double * pTempBuffer);
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_SendCommand(IR_PLUG_SDK_CommandTypeDef cmd, unsigned short value);
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_SetPallete(IR_PLUG_SDK_ColormapTypeDef type);
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_DeInit(void);
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_SetLeptonType(U08);
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_SetEmissivity(F32);
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_SetFpaTemp(F32, U32);
extern const char * IR_PLUG_SDK_GetErrorText(void);
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_SetFlashingPeriod(U08 onSec, U08 offSec);

///< 获取原始图像数据缓冲区指针
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_GetFrameRawBuffer(unsigned short ** pFrameRawBuffer);



/* ------------------------------------------------------------------------------ */





///< 唤醒
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_WakeUp();

///< 休眠
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_Sleep();

///< 获取设备参数
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_GetDeviceInfo(IR_PLUG_SDK_DeviceInfoTypeDef * pInfo);

///< Shutter校准
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_ShutterCalibration();

///< 获取一副图像（返回RGB，返回处理状态）
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_ReadImageRgb(U32 * imgBuffer, U32 buffSize);


///< 捕获一副图像（传入图像个数，在按图像保存后，返回一个温度数组，一幅RGB图像，返回状态）
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_Capture(U32* imgBuffer, U32 imgBuffSize, F32* tempBuffer, U32 tempBuffSize);

///< 进入温度精度校准模式
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_EnterTempAccuracyCalibrationMode();
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_AbortTempAccuracyCalibrationMode();

///< 进入温度精准验证模式
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_EnterTempAccuracyVerifyMode();

///< 温度一致性校准模式
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_EnterTempUniformityCalibrationMode();
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_AbortTempUniformityCalibrationMode();

///< 温度一致性验证模式
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_EnterTempUniformityVerifyMode();


///< 获取设备状态
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_GetDeviceState(U08 * state);

///< 触发TAC开始采集信号
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_StartTacCapture(void);

///< 调试信息输出
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_EnableDebug(U08 enable);

///< 获取指定点的温度值
extern IR_PLUG_SDK_ErrorTypeDef IR_PLUG_SDK_GetTemp(int x, int y, F32 * temp);







