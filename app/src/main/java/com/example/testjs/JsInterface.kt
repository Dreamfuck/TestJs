package com.example.myapplication

import android.content.Context
import android.util.Log
import android.webkit.JavascriptInterface
import com.example.testjs.App
import com.example.testjs.toast

class JsInterface() {

    @JavascriptInterface
    fun test1(){
        Log.i("this is js test","no p")
    }

    @JavascriptInterface
    fun test2(data : String){
        data.toast(App.instance).show()
        Log.i("this is js test",data)
    }

}