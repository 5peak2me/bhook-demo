#include <jni.h>
#include <string>
#include <cinttypes>
#include <bytehook.h>
#include "log.h"

void
getapi_hooked_callback(bytehook_stub_t task_stub, int status_code, const char *caller_path_name,
                       const char *sym_name, void *new_func, void *prev_func, void *arg) {
    if (BYTEHOOK_STATUS_CODE_ORIG_ADDR == status_code) {
        ALOGE(">>>>> save original address: %" PRIxPTR, (uintptr_t) prev_func);
    } else {
        ALOGE(">>>>> hooked. stub: %" PRIxPTR", status: %d, caller_path_name: %s, sym_name: %s, new_func: %" PRIxPTR", prev_func: %" PRIxPTR", arg: %" PRIxPTR,
              (uintptr_t) task_stub, status_code, caller_path_name, sym_name, (uintptr_t) new_func,
              (uintptr_t) prev_func, (uintptr_t) arg);
    }
}

std::string getapi_new() {
    BYTEHOOK_STACK_SCOPE();
    ALOGE("%s", "this is a hook method 之前");
    std::string res = BYTEHOOK_CALL_PREV(getapi_new);

    ALOGD("============> 在这里做了很多事情，制造了成吨的伤害 <==========");

    ALOGE("%s", "this is a hook method 之后");
    return res;
}

bytehook_stub_t stub = nullptr;

extern "C" JNIEXPORT void JNICALL
Java_com_l3gacy_app_bhook_NativeLib_hook(JNIEnv *env, jobject thiz, jint type) {
    (void) env, (void) thiz;

    if (type == 0) {
        stub = bytehook_hook_single("libtest.so",
                                    nullptr,
                                    "_Z6getapiv",
                                    (void *) (getapi_new),
                                    getapi_hooked_callback,
                                    nullptr);
    } else if (type == 1) {

    } else {
        stub = bytehook_hook_all(nullptr,
                                 "getapi",
                                 reinterpret_cast<void *>(getapi_new),
                                 nullptr,
                                 nullptr);
    }

}

extern "C" JNIEXPORT void JNICALL
Java_com_l3gacy_app_bhook_NativeLib_unhook(JNIEnv *env, jobject thiz) {
    (void) env, (void) thiz;

    if (nullptr != stub) {
        bytehook_unhook(stub);
        stub = nullptr;
    }
}

