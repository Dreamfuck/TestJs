package com.example.testjs

import android.content.Context
import android.widget.Toast

fun String.toast(context : Context, direct : Int = Toast.LENGTH_SHORT) : Toast{
    return Toast.makeText(context,this,direct)
}


