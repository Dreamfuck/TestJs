package com.example.myapplication

import android.content.Context
import android.content.DialogInterface
import android.net.Uri
import android.os.Build
import android.util.Log
import android.webkit.*
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AlertDialog

/**
 * 拦截弹出框 prompt
 */
class MyWebChromeClient(val context: Context) : WebChromeClient() {

    /**
     * 无返回值
     */
    override fun onJsAlert(
        view: WebView?,
        url: String?,
        message: String?,
        result: JsResult?
    ): Boolean {
        val b: AlertDialog.Builder = AlertDialog.Builder(context)
        b.setTitle("Alert")
        b.setMessage(message)
        b.setPositiveButton(
            android.R.string.ok,
            DialogInterface.OnClickListener { dialog, which -> result?.confirm() })
        b.setCancelable(false)
        b.create().show()
        return true
    }

    /**
     * 返回参数
     */
    override fun onJsPrompt(
        view: WebView?,
        url: String?,
        message: String?,
        defaultValue: String?,
        result: JsPromptResult?
    ): Boolean {
        if (message.equals("getData")){
            result?.confirm("这是一个带参数的弹窗测试");
            return true;
        }
        return super.onJsPrompt(view, url, message, defaultValue, result);

    }

    // For Android 5.0+
    @RequiresApi(Build.VERSION_CODES.LOLLIPOP)
    override fun onShowFileChooser(
        webView: WebView?,
        filePathCallback: ValueCallback<Array<Uri?>?>,
        fileChooserParams: FileChooserParams?
    ): Boolean {
        Log.i("openFileChoose","onShowFileChooser")
        Log.i("openFileChoose", "${fileChooserParams?.acceptTypes!![0]}")


        return true
    }


}