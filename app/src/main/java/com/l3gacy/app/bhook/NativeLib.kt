package com.l3gacy.app.bhook

object NativeLib {

    init {
        System.loadLibrary("bhookdemo")
    }

    external fun hook(type: Int = 0)

    external fun unhook()

    external fun dump(pathname: String)

}