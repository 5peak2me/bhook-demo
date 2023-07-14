package com.l3gacy.app.bhook;

import com.bytedance.android.bytehook.ByteHook;

import androidx.annotation.NonNull;

public class ThreadHook {
    static {
        System.loadLibrary("bhookdemo");
    }

    private static boolean sHasHook = false;

    public static synchronized void init() {
        ByteHook.init();
    }

    public static String getStack() {
        String d = null;
        try {
//            d = stackTraceToString(new Throwable().getStackTrace());
            d = stackTraceToString(Thread.currentThread().getStackTrace());
        } catch (Exception ignored) {

        }
        return d;
    }

    @NonNull
    private static String stackTraceToString(final StackTraceElement[] arr) {
        if (arr == null) {
            return "";
        }

        StringBuilder sb = new StringBuilder();

        for (StackTraceElement stackTraceElement : arr) {
            String className = stackTraceElement.getClassName();
            // remove unused stacks
            if (className.contains("java.lang.Thread")) {
                continue;
            }

            sb.append(stackTraceElement).append('\n');
        }
        return sb.toString();
    }

    public static void enableThreadHook() {
        if (sHasHook) {
            return;
        }
        sHasHook = true;
        enableThreadHookNative();
    }

    public static void unableThreadHook() {
        if (!sHasHook) {
            return;
        }
        sHasHook = false;
        nativeUnhook();
    }


    private static native int enableThreadHookNative();

    private static native int nativeUnhook();
}