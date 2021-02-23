package com.example.testjs;

import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import net.launchdigital.irsdk.IrSdk;

import java.util.List;


/**
 * author: ztj
 * date: 2021/1/15
 * des:
 */
public class TestAct extends AppCompatActivity {

    private IrSdk mIrSdk = null;
    private static final String TAG = "MainWnd";
    private static final String TAG1 = "APPpackgename";

    private Handler theHandler = null;
    private int[] mImageBuffer;
    private double[] mTempBuffer;
    private Bitmap mBitmap;
    private TextView tv = null;
    private ImageView mImageView = null;

    private int count = 0, color = 0;

    private String DoubleToStr(double val)
    {
        return (String.format("%.2f℃",val));
    }

    private final Runnable mRefreshThread = new Runnable() {

        @Override
        public void run() {

            if (null != mIrSdk) {
                Log.v(TAG, "mIrSdkonCreate");
                if (0 < mIrSdk.captureImage(
                        mImageBuffer, 160 * 120 * Integer.BYTES,
                        mTempBuffer, 160 * 120 * Double.BYTES)) {
                    Log.v(TAG, "mIrSdkcaptureImageonCreate");
                    mBitmap.setPixels(mImageBuffer, 0, 120, 0, 0, 120, 160);

                    mImageView.setImageBitmap(mBitmap);
                    tv.setText(DoubleToStr(mTempBuffer[80 * 120 + 60]));
                }
            }
            theHandler.postDelayed(this, 10);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.zhanlan);
        Intent intent = new Intent();
        String str;
        if (true) {
            str = "agui.intent.action.USB_CAMREA_OPEN";
        } else {
            str = "agui.intent.action.USB_CAMREA_CLOSE";
        }
        intent.setAction(str);
        sendBroadcast(intent);
        mImageBuffer = new int[160 * 120];
        mTempBuffer = new double[320 * 240];
        mBitmap = Bitmap.createBitmap(120, 160, Bitmap.Config.ARGB_8888);

        mImageView = findViewById(R.id.iv);
        tv =  findViewById(R.id.tv);

        mIrSdk = new IrSdk(this);

        theHandler = new Handler();
        theHandler.postDelayed(mRefreshThread, 5000);


    }

    public void getAllAppNames() {
        PackageManager pm = getPackageManager();
        int j = 0;
        List<PackageInfo> list2 = pm.getInstalledPackages(PackageManager.GET_UNINSTALLED_PACKAGES);
        for (PackageInfo packageInfo : list2) {
            //得到手机上已经安装的应用的名字,即在AndriodMainfest.xml中的app_name。
            String appName = packageInfo.applicationInfo.loadLabel(getPackageManager()).toString();
            //得到应用所在包的名字,即在AndriodMainfest.xml中的package的值。
            String packageName = packageInfo.packageName;
            Log.i(TAG1, "应用的名字:" + appName);
            Log.i(TAG1, "应用的包名字:" + packageName);

            j++;
        }
        Log.i(TAG1, "应用的总个数:" + j);
    }

    @Override
    protected void onStart() {
        super.onStart();

        if (null != mIrSdk) {
            mIrSdk.register();
        }

        Log.v(TAG, "onStart");
    }

    @Override
    protected void onStop() {
        super.onStop();

        if (null != mIrSdk) {
            mIrSdk.unregister();
        }
        Log.v(TAG, "onStop");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (null != mIrSdk) {
            mIrSdk.destroy();
        }

        Log.v(TAG, "onDestroy");
    }
}