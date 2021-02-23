#ifndef _IR_PLUG_DEVICE_
#define _IR_PLUG_DEVICE_

#include "ir_platform.h"

/**
  * @brief USB 操作返回结果
  */
typedef enum {
  IR_PLUG_DEV_SUCCESS = -100,       /*!< 成功               */
  IR_PLUG_DEV_INVALID_HANDLE,          /*!< 提供的句柄无效     */
  IR_PLUG_DEV_USBCORE,                 /*!< USB内核错误        */
} IR_PLUG_DEV_ErrorTypeDef;

/**
  * @brief IR PLUG 设备类请求代码，用于控制传输中的 bRequest 字段，
  *        下位机需要实现每一种对应的操作请求
  */
typedef enum {

  IR_PLUG_DEV_GET_CFG,                        /*!< 设备配置请求           */
  IR_PLUG_DEV_GET_STATUS,
  IR_PLUG_DEV_CLR_STATUS,
  IR_PLUG_DEV_GET_STATE
} IR_PLUG_DEV_RequestTypeDef;

/**
  * @brief 底层 USB 操作接口
  */
typedef struct tagIR_PLUG_DEV_Interface {
  /*!< USB 初始化                         */
  IR_PLUG_DEV_ErrorTypeDef (*init)(const char *, int, int, int);

  /*!< USB 初始化                         */
  IR_PLUG_DEV_ErrorTypeDef (*deinit)();

  /*!< 控制命令：读数据                   */
  IR_PLUG_DEV_ErrorTypeDef (*controlRead)(unsigned char, unsigned short, unsigned char *, int);

  /*!< 控制命令：写数据                   */
  IR_PLUG_DEV_ErrorTypeDef (*controlWrite)(unsigned char, unsigned short, unsigned char *, int);

  /*!< 批量读取数据                       */
  IR_PLUG_DEV_ErrorTypeDef (*bulkRead)(unsigned char *, int);

  /*!< 批量写数据*/
  IR_PLUG_DEV_ErrorTypeDef (*bulkWrite)(unsigned char *, int);

  /*!< 获取错误描述字符串信息             */
  const char *(*errorText)(void);
} IR_PLUG_DEV_Interface;

extern IR_PLUG_DEV_ErrorTypeDef IR_PLUG_DEV_Init(const char *deviceName,
                                                 int deviceFileDescriptor,
                                                 int,
                                                 int);
extern IR_PLUG_DEV_ErrorTypeDef IR_PLUG_DEV_Deinit();
extern IR_PLUG_DEV_ErrorTypeDef IR_PLUG_DEV_ControlRead(unsigned char bRequest,
                                                        unsigned short wValue,
                                                        unsigned char *data,
                                                        int size);
extern IR_PLUG_DEV_ErrorTypeDef IR_PLUG_DEV_ControlWrite(unsigned char bRequest,
                                                         unsigned short wValue,
                                                         unsigned char *data,
                                                         int size);
extern IR_PLUG_DEV_ErrorTypeDef IR_PLUG_DEV_BulkRead(unsigned char *data, int size);
extern IR_PLUG_DEV_ErrorTypeDef IR_PLUG_DEV_BulkWrite(unsigned char *data, int size);
extern const char *IR_PLUG_DEV_GetErrorText(void);

#endif



