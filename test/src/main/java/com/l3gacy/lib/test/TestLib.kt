package com.l3gacy.lib.test

class TestLib {

    /**
     * A native method that is implemented by the 'test' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'test' library on application startup.
        init {
            System.loadLibrary("test")
        }
    }
}