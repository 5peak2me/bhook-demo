#include <jni.h>
#include <string>
#include <android/log.h>

std::string getapi() {
    int api = android_get_device_api_level();
    std::string hello = "Hello from C++, running on Android " + std::to_string(api);
    return hello;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_l3gacy_lib_test_TestLib_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string res = getapi();
    return env->NewStringUTF(res.c_str());
}