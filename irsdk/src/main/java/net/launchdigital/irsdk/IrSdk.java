package net.launchdigital.irsdk;


import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Handler;
import android.text.TextUtils;
import android.util.Log;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Iterator;

public final class IrSdk {
    private static final int IR_PLUG_VID = 0x0483;
    private static final int IR_PLUG_PID = 0x8080;

    private static final String ACTION_USB_PERMISSION = "net.launchdigital.USB_PERMISSION";
    private static final String TAG = "IrSdk";

    private final UsbManager mUsbManager;
    private final WeakReference<Context> mWeakContext;


    /**
     * 当前操作的 USB 设备
     */
    private UsbDevice mUsbDevice = null;

    /**
     * USB 设备连接状态
     */
    private volatile boolean mUsbDeviceConnected = false;

    /**
     * SDK 初始化状态
     */
    private volatile boolean mIrSdkReady = false;
    private volatile boolean mDestroyed = true;
    private PendingIntent mPermissionIntent = null;


    /**
     * 异步任务调度
     */
    private Handler mTheHandler = null;

    /**
     * 硬件 USB 连接检测线程
     */
    private final Runnable mUsbMonitorThread = new Runnable() {
        @Override
        public void run() {
            if (!mUsbDeviceConnected) {
                // 不断检测 USB 是否连接
                checkUsbConnection();
            } else if (!mIrSdkReady && (null != mUsbDevice)) {
                // USB 已连接，但是，SDK尚未初始化成功
                // 不断进行初始化
                synchronized (this) {
                    processConnect(mUsbDevice);
                }
            }
            mTheHandler.postDelayed(this, 1000);
        }
    };


    public IrSdk(final Context context) {
        mWeakContext = new WeakReference<Context>(context);
        mUsbManager = (UsbManager) context.getSystemService(Context.USB_SERVICE);

        mUsbDeviceConnected = false;
        mDestroyed = false;
        mIrSdkReady = false;
        mTheHandler = new Handler();
    }


    /**
     * 注册系统监听信息等，必须在 Activity 的 onStart 中调用
     *
     * @throws IllegalStateException
     */
    public synchronized void register() throws IllegalStateException {

        if (mDestroyed) {
            throw new IllegalStateException("already destroyed");
        }

        if (null == mPermissionIntent) {
            // 注册广播回调接口
            final Context context = mWeakContext.get();
            if (context != null) {
                mPermissionIntent = PendingIntent.getBroadcast(context, 0, new Intent(ACTION_USB_PERMISSION), 0);
                final IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);

                // ACTION_USB_DEVICE_ATTACHED never comes on some devices so it should not be added here
                filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
                filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
                context.registerReceiver(mUsbReceiver, filter);
            }

            // 启动线程，检测 USB 连接
            mTheHandler.postDelayed(mUsbMonitorThread, 1000);
        }
    }


    /**
     * 取消注册，必须在 Activity 的 onStop 中调用
     *
     * @throws IllegalStateException
     */
    public synchronized void unregister() throws IllegalStateException {
        // 销毁 SDK 相关资源
        if (mIrSdkReady) {
            if (0 != IrSdkDeinit()) {
                Log.v(TAG, "IrSdkDeinit error!");
            }

            mIrSdkReady = false;

            mUsbDevice = null;
        }

        // 下位机断电
        mUsbDeviceConnected = false;

        // 结束 USB 连接检测线程
        if (!mDestroyed) {
            mTheHandler.removeCallbacks(mUsbMonitorThread);
        }

        // 销毁权限请求等资源
        if (mPermissionIntent != null) {
            final Context context = mWeakContext.get();
            try {
                if (context != null) {
                    context.unregisterReceiver(mUsbReceiver);
                }
            } catch (final Exception e) {
                e.printStackTrace();
            }
            mPermissionIntent = null;
        }
    }

    /**
     * 销毁资源等，必须在 Activity 的 onDestroy 调用
     */
    public void destroy() {
        unregister();
        if (!mDestroyed) {
            mDestroyed = true;

            /* 关闭系统 USB 连接信息，释放资源                   */
            if (null != mUsbDeviceConnection) {
                mUsbDeviceConnection.close();
                mUsbDeviceConnection = null;
            }

            try {
                mTheHandler.getLooper().quit();
            } catch (final Exception e) {
                Log.e(TAG, "destroy: ", e);
            }
        }
    }

    /**
     * 设备信息参数
     */
    public static class DeviceInfo {
//        public String firmwareVersion;      /*!< 模组固件版本信息，格式 x.x.x         */
//        public int sensorVersion;        /*!< 所使用的传感器版本，2或3              */
//        public String tacDatetime;          /*!< TAC校准日期时间20170719 180101       */
//        public String tucDatetime;          /*!< TUC校准日期时间20170719 180101       */

        public String sdkVersion;       /*!< 当前 SDK 版本信息，格式 x.x.x */
        public int  tacParamValid;             /*!< TAC 校准参数是否有效 */
        public String tacDatetime;           /*!< TAC校准日期时间20170719 180101              */
        public int  tucParamValid;             /*!< TUC 校准参数是否有效 */
        public String tucDatetime;           /*!< TUC校准日期时间20170719 180101              */
        public int hardwareInfoSource;         /*!< 模组硬件信息来源 */

        /*!< 模组硬件相关信息 */
        public int  leptonType;            // Lepton 型号
        public String leptonId;          // Lepton 序列号
        public String firmwareVersion;    // 固件版本号：1.0.0，2.0.1等
        public String hardwareId;        // CPU ID
    }

    public int getDeviceInfo(DeviceInfo devInfo) {
        return (IrSdkGetDeviceInfo(devInfo));
    }


    public int getDeviceState() {
        return (IrSdkGetDeviceState());
    }

    public int readImage(int[] imgBuffer, int buffSize) {
        int ret = 0;

        if (!mIrSdkReady) {
            Log.v(TAG, "irsdk not inited, readFrameRgb failed!");
            return (-1);
        }

        ret = IrSdkReadFrameRgb(imgBuffer, buffSize);

        return (ret);
    }

    public int captureImage(int[] imgBuffer, int imgBuffSize,
                            double[] tempBuffer, int tempBuffSize) {

        int ret = 0;

        if (!mIrSdkReady) {
            Log.e(TAG, "irsdk not inited, readFrameRgb failed!");
            return (-1);
        }

        ret = IrSdkCaptureFrame(imgBuffer, imgBuffSize, tempBuffer, tempBuffSize);

        return (ret);
    }

    public int enterTempAccuracyCalibrationMode() {
        return (IrSdkEnterTempAccuracyCalibrationMode());
    }

    public int abortTempAccuracyCalibrationMode() {
        return (IrSdkAbortTempAccuracyCalibrationMode());
    }

    public int enterTempUniformityCalibrationMode() {
        return (IrSdkEnterTempUniformityCalibrationMode());
    }

    public int abortTempUniformityCalibrationMode() {
        return (IrSdkAbortTempUniformityCalibrationMode());
    }

    public int startTacCapture() {
        return (IrSdkStartTacCapture());
    }

    public int setLeptonType(int type) {
        return IrSdkSetLeptonType(type);
    }
    public int setEmissivity(double emissivity) {return IrSdkSetEmissivity(emissivity); }

    public boolean isIrSdkReady() {
        return ((false == mDestroyed) && (true == mIrSdkReady));
    }

    public void enableDebug(boolean enable) {
        if (enable) {
            IrSdkEnableDebug(1);
        } else {
            IrSdkEnableDebug(0);
        }
    }

    public int setColormap(int type) {
        if (!mIrSdkReady) {
            Log.v(TAG, "Ir sdk is not ready!");
            return(-1);
        }

        return (IrSdkSetColormap(type));
    }

    public int shutterCalibration() {
        if (!mIrSdkReady) {
            Log.v(TAG, "Ir sdk is not ready!");
            return(-1);
        }

        return (IrSdkShutterCalibration());
    }


    /* 内部接口 ----------------------------------------------------------------------------------*/
    private int checkUsbConnection() {
        // 枚举当前所有设备，找到 IR 设备
        UsbDevice irDevice = null;

        HashMap<String, UsbDevice> deviceList = mUsbManager.getDeviceList();
        Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
        while (deviceIterator.hasNext()) {
            UsbDevice itemDev = deviceIterator.next();
            if (itemDev.getVendorId() == IR_PLUG_VID
                    && itemDev.getProductId() == IR_PLUG_PID) {
                irDevice = itemDev;
                break;
            }
        }

        if (null != irDevice) {
            // 设备已连接，并枚举成功，请求读写权限
            mUsbManager.requestPermission(irDevice, mPermissionIntent);
            mUsbDeviceConnected = true;
        }

        return (0);
    }


    /**
     * BroadcastReceiver for USB permission
     */
    private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(final Context context, final Intent intent) {
            final String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                // when received the result of requesting USB permission
                synchronized (this) {
                    final UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        if (device != null) {
                            mUsbDevice = device;
                            processConnect(mUsbDevice);
                        }
                    } else {
                    }
                }
            } else if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
            } else if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
            }
        }
    };


    private UsbDeviceConnection mUsbDeviceConnection = null;

    private void processConnect(final UsbDevice device) {
        if (null != mUsbDeviceConnection) {
            mUsbDeviceConnection.close();
            mUsbDeviceConnection = null;
        }
        mUsbDeviceConnection = mUsbManager.openDevice(device);
        String mUsbDeviceName = device.getDeviceName();
        int mUsbFileDescriptor = mUsbDeviceConnection.getFileDescriptor();

        int usbBusnum = 0;
        int usbDevnum = 0;
        final String[] v = !TextUtils.isEmpty(mUsbDeviceName) ? mUsbDeviceName.split("/") : null;
        if (v != null) {
            usbBusnum = Integer.parseInt(v[v.length - 2]);
            usbDevnum = Integer.parseInt(v[v.length - 1]);
        }

        final Context context = mWeakContext.get();
        if (null != context) {
            int ret = 0;
            ret = IrSdkSetWorkDir(context.getFilesDir().getPath());
            if (0 != ret) {
                return;
            }
            ret = IrSdkInit(mUsbDeviceName, mUsbFileDescriptor, usbBusnum, usbDevnum);
            mIrSdkReady = (0 == ret);
        }
    }


    /* JNI native 接口部分 ------------------------------------------------------------------------ */
    private static boolean isLoaded;

    static {
        if (!isLoaded) {
            System.loadLibrary("IrSdk");
            isLoaded = true;
        }
    }

    private native int IrSdkDeinit();

    private native int IrSdkInit(String deviceName, int deviceFileDescriptor, int busNum, int devNum);

    private native int IrSdkReadFrameRgb(int[] buffer, int size);

    private native int IrSdkCaptureFrame(int[] imgBuffer, int imgBuffSize,
                                         double[] tempBuffer, int tempBuffSize);

    private native int IrSdkSetWorkDir(String dir);

    private native int IrSdkGetDeviceInfo(DeviceInfo deviceInfo);

    private native int IrSdkGetDeviceState();

    private native int IrSdkShutterCalibration();

    private native int IrSdkEnterTempAccuracyCalibrationMode();

    private native int IrSdkAbortTempAccuracyCalibrationMode();

    private native int IrSdkEnterTempUniformityCalibrationMode();

    private native int IrSdkAbortTempUniformityCalibrationMode();

    private native int IrSdkStartTacCapture();

    private native int IrSdkSetLeptonType(int type);

    private native int IrSdkSetEmissivity(double emissivity);

    private native int IrSdkEnableDebug(int enable);

    private native double IrSdkGetCurrentTemp(int x, int y);

    private native int IrSdkSetColormap(int type);
}




























