package com.l3gacy.app.bhook

import android.app.Application
import android.content.Context
import android.os.StrictMode
import androidx.viewbinding.BuildConfig
import com.bytedance.android.bytehook.ByteHook


class App: Application() {

    override fun attachBaseContext(base: Context?) {
        super.attachBaseContext(base)

        ByteHook.init()
        ByteHook.setDebug(true)
        System.loadLibrary("test")
        System.loadLibrary("detour")
    }

    override fun onCreate() {
        super.onCreate()

//        init()

    }

    private fun init() {
        if (BuildConfig.DEBUG.not()) return
        StrictMode.setThreadPolicy(
            StrictMode.ThreadPolicy.Builder()
                .detectDiskReads()
                .detectDiskWrites()
                .detectNetwork() // or .detectAll() for all detectable problems
                .penaltyLog()
                .build()
        )
        StrictMode.setVmPolicy(
            StrictMode.VmPolicy.Builder()
                .detectLeakedSqlLiteObjects()
                .detectLeakedClosableObjects()
                .penaltyLog()
//                .penaltyDeath()
                .build()
        )
    }

}