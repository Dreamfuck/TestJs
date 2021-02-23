package com.example.testjs

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.webkit.ValueCallback
import android.webkit.WebChromeClient
import android.webkit.WebSettings
import android.webkit.WebView
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import com.example.myapplication.JsInterface
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    @RequiresApi(Build.VERSION_CODES.KITKAT)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        initWebView()

        //调用js 效率高 4.4以上
        bt.setOnClickListener {
            web.evaluateJavascript("javascript:callJs('123')",object : ValueCallback<String> {
                override fun onReceiveValue(str: String?) {
                    Log.i("this is js test","js result $str")
                }
            })
        }

        bt1.setOnClickListener {
            startActivity(Intent(this,TestAct::class.java))
        }

    }

    /**
     * 初始化WebView
     */
    fun initWebView(){
        val webSet : WebSettings = web.settings

        //支持js交互操作
        webSet.javaScriptEnabled = true
        webSet.javaScriptCanOpenWindowsAutomatically = true
        webSet.loadsImagesAutomatically = true

        web.loadUrl("file:///android_asset/t.html")

        //设置js调用android
        web.addJavascriptInterface(JsInterface(),"testJs")

        //设置js调用
        web.webChromeClient = MyTakeWebChromeClient(this)

    }

    class MyTakeWebChromeClient(val context: Activity) : WebChromeClient() {

        // For Android 5.0+
        @RequiresApi(Build.VERSION_CODES.LOLLIPOP)
        override fun onShowFileChooser(
            webView: WebView?,
            filePathCallback: ValueCallback<Array<Uri?>?>,
            fileChooserParams: FileChooserParams?
        ): Boolean {
            Log.i("openFileChoose","onShowFileChooser")
            Log.i("openFileChoose", "${fileChooserParams?.acceptTypes!![0]}")
            val intent = Intent(context,TestAct::class.java)
            context.startActivityForResult(intent, 1011)
            return true
        }

    }

}
