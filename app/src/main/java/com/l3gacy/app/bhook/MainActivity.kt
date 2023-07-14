package com.l3gacy.app.bhook

import android.content.Context
import android.os.BatteryManager
import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import com.l3gacy.app.bhook.databinding.ActivityMainBinding
import com.l3gacy.lib.test.TestLib
import kotlin.concurrent.thread


class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        with(binding) {

            // #########################################
            //   自有动态库 added at 2022/10/9
            // #########################################

            btnHook1.setOnClickListener {
                NativeLib.hook()
            }

            btnUnhook1.setOnClickListener {
                NativeLib.unhook()
            }

            btnCall1.setOnClickListener {
                Log.d(TAG, TestLib().stringFromJNI())
            }

            // #########################################
            //   自有动态库 added at 2022/10/9
            // #########################################


            // #########################################
            //   系统动态库 added at 2022/10/9
            // #########################################

            btnHook2.setOnClickListener {
                ThreadHook.enableThreadHook()
            }
            btnCall2.setOnClickListener {
//                println(ThreadHook.getStack())
                thread {
                    Log.w(
                        TAG,
                        "thread name: ${Thread.currentThread().name}; " + "thread id: ${Thread.currentThread().id}"
                    )
                    thread {
                        Log.w(
                            TAG,
                            "inner thread name: ${Thread.currentThread().name}; " + "inner thread id: ${Thread.currentThread().id}"
                        )
                    }
                }
            }
            btnUnhook2.setOnClickListener {
                ThreadHook.unableThreadHook()
            }

            // #########################################
            //   系统动态库 added at 2022/10/9
            // #########################################

            btnHook3.setOnClickListener {
                NativeHacker.hook(0)
            }

            btnCall3.setOnClickListener {
//                println(filesDir.absolutePath)
                val manager = getSystemService(Context.BATTERY_SERVICE) as BatteryManager
                if (manager == null) {
                    throw NullPointerException("Cannot get BluetoothManager")
                } else {
                    val isCharging = manager.isCharging
                    Log.d(TAG, "isCharging: $isCharging")
                }
            }

            btnUnhook3.setOnClickListener {
                NativeHacker.unhook()
            }

        }

    }

    companion object {
        private const val TAG: String = "hook_tag"
    }

}