/* Includes ------------------------------------------------------------------*/
#include "ir_plug_device.h"

#ifdef WIN32
#include "libusb_win32/include/lusb0_usb.h"
#else
#include "libusb.h"
#endif





/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static libusb_context *g_pLibUsbContext = NULL;
static libusb_device  *g_pLibUsbDevice = NULL;
static libusb_device_handle *g_hDev = NULL;
static char g_strBuff[1024] = {0};

/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/

/* 设备 ID 信息                 */
#define IR_PLUG_DEV_VID             (0x0483)
#define IR_PLUG_DEV_PID             (0x8080)//(0x1234)

/* USB 端点、配置和接口信息           */
#define IR_PLUG_DEV_CONFIG          (1)
#define IR_PLUG_DEV_INTERFACE       (0)
#define IR_PLUG_DEV_BULK_IN         (0x81)
#define IR_PLUG_DEV_BULK_OUT        (0x01)

/* 调试模拟数据                 */
//#define IR_PLUG_DEV_USB_DEBUG

/* Private functions ---------------------------------------------------------*/
static libusb_device_handle *_OpenDevice(unsigned short vid, unsigned short pid);

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
IR_PLUG_DEV_ErrorTypeDef IR_PLUG_DEV_Init(const char *deviceName,
        int deviceFileDescriptor,
        int busNum,
        int devNum)
{
    srand((unsigned int) time(0));
    memset(g_strBuff, 0, sizeof(g_strBuff));

    int r = 0;

#if defined(IR_PLUG_DEV_USB_DEBUG)
    return (IR_PLUG_DEV_SUCCESS);
#else

#if 1   // uvccamera libusb
    r = libusb_init2(&g_pLibUsbContext, deviceName);

    if (0 != r)
    {
        return (IR_PLUG_DEV_USBCORE);
    }

    g_pLibUsbDevice = libusb_get_device_with_fd(g_pLibUsbContext,
                      IR_PLUG_DEV_VID, IR_PLUG_DEV_PID, NULL,
                      deviceFileDescriptor, busNum, devNum);

    r = libusb_open(g_pLibUsbDevice, &g_hDev);

    if (0 != r)
    {
        return (IR_PLUG_DEV_USBCORE);
    }

#endif

    /* 启用设备指定的 配置 和 接口 */
    if (libusb_set_configuration(g_hDev, IR_PLUG_DEV_CONFIG) < 0
            || libusb_claim_interface(g_hDev, IR_PLUG_DEV_INTERFACE) < 0)
    {
        libusb_close(g_hDev);
        return (IR_PLUG_DEV_USBCORE);
    }

    return (IR_PLUG_DEV_SUCCESS);
#endif

}

IR_PLUG_DEV_ErrorTypeDef IR_PLUG_DEV_Deinit()
{

    libusb_release_interface(g_hDev, IR_PLUG_DEV_INTERFACE);

    //  if (libusb_set_configuration(g_hDev, IR_PLUG_DEV_CONFIG) < 0
    //      || libusb_claim_interface(g_hDev, IR_PLUG_DEV_INTERFACE) < 0) {


    if (NULL != g_hDev)
    {
        /* 关闭设备句柄                       */
        libusb_close(g_hDev);
    }

    if (NULL != g_pLibUsbContext)
    {
        /* 释放 libusb 相关资源               */
        libusb_exit(g_pLibUsbContext);
        g_pLibUsbContext = NULL;
    }

    return (IR_PLUG_DEV_SUCCESS);
}

IR_PLUG_DEV_ErrorTypeDef IR_PLUG_DEV_ControlRead(
    unsigned char bRequest,
    unsigned short wValue,
    unsigned char *data, int size)
{
#if defined(IR_PLUG_DEV_USB_DEBUG)
    return (IR_PLUG_DEV_SUCCESS);
#else

    int ret = 0;

    ret = libusb_control_transfer(g_hDev,
                                  LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS
                                  | LIBUSB_RECIPIENT_ENDPOINT,
                                  bRequest,
                                  wValue, 0, (unsigned char *) data, (uint16_t) size, 1000
                                 );

    if (ret < 0)
    {
        snprintf(g_strBuff, sizeof(g_strBuff), "%s", libusb_error_name(ret));
        return (IR_PLUG_DEV_USBCORE);
    }

    return (IR_PLUG_DEV_SUCCESS);
#endif
}

IR_PLUG_DEV_ErrorTypeDef IR_PLUG_DEV_ControlWrite(
    unsigned char bRequest,
    unsigned short wValue,
    unsigned char *data, int size)
{
#if defined(IR_PLUG_DEV_USB_DEBUG)
    return (IR_PLUG_DEV_SUCCESS);
#else

    int ret = 0;
    int temp = 0xFF;

    ret = libusb_control_transfer(g_hDev,
                                  LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS
                                  | LIBUSB_RECIPIENT_ENDPOINT,
                                  bRequest,
                                  wValue, 0, (unsigned char *) data, (uint16_t) size, 1000);

    if (ret < 0)
    {
        snprintf(g_strBuff, sizeof(g_strBuff), "%s", libusb_error_name(ret));
        return (IR_PLUG_DEV_USBCORE);
    }

    return (IR_PLUG_DEV_SUCCESS);

#endif
}

IR_PLUG_DEV_ErrorTypeDef IR_PLUG_DEV_BulkRead(unsigned char *data, int size)
{
#if defined(IR_PLUG_DEV_USB_DEBUG)
    static int idx = 0;
    unsigned short *pData = (unsigned short *)data;
    size >>= 1;

    for (int i = 0; i < size; ++i)
    {
        //         pData[i] = 2480 + rand() % (4530 - 2480);
        //         pData[i] = 2480 + (i + idx) % (4530 - 2480);
        //         pData[i] = 2480 + (i) % (4530 - 2480);
        //         pData[i] = 2480 + i % 160;
        pData[i] = (unsigned short)((i) % 16384);
    }

    idx++;

    unsigned short(*pData2)[19200] = (unsigned short(*)[19200])pData;
    pData2 = pData2;

#else

    int ret = 0, bulkOLen = 0;

    ret = libusb_bulk_transfer(g_hDev, IR_PLUG_DEV_BULK_IN,
                               (unsigned char *)data, size, &bulkOLen, 5000);

    if (ret < 0)
    {
        snprintf(g_strBuff, sizeof(g_strBuff), "%s", libusb_error_name(ret));
        return (IR_PLUG_DEV_USBCORE);
    }

#endif
    return (IR_PLUG_DEV_SUCCESS);
}


IR_PLUG_DEV_ErrorTypeDef IR_PLUG_DEV_BulkWrite(unsigned char *data, int size)
{
    return (IR_PLUG_DEV_SUCCESS);
}


const char *IR_PLUG_DEV_GetErrorText(void)
{
    return (g_strBuff);
}