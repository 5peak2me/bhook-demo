#include <cstdlib>
#include <unistd.h>
#include <cstdint>
#include <cinttypes>
#include <cstring>
#include <dlfcn.h>
#include <fcntl.h>
#include <jni.h>
#include <ctime>
#include <android/log.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>

#include <pthread.h>
#include "bytehook.h"


#define HACKER_JNI_CLASS_NAME "com/l3gacy/app/bhook/ThreadHook"
#define HACKER_TAG            "hook_tag"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#define LOG(fmt, ...)  __android_log_print(ANDROID_LOG_INFO, HACKER_TAG, fmt, ##__VA_ARGS__)
#pragma clang diagnostic pop


typedef int (*pthread_p)(pthread_t *, pthread_attr_t const *, void *(*__start_routine)(void *),
                         void *);

static jclass kJavaClass;
static jmethodID kMethodGetStack;
static JavaVM *kJvm;
static pthread_p pthread_p_prev = nullptr;
static bytehook_stub_t pthread_p_stub = nullptr;

static void
pthread_p_hooked_callback(bytehook_stub_t task_stub, int status_code, const char *caller_path_name,
                          const char *sym_name, void *new_func, void *prev_func, void *arg) {
    if (BYTEHOOK_STATUS_CODE_ORIG_ADDR == status_code) {
        pthread_p_prev = (pthread_p) prev_func;
        LOG("hook_tag >>>>> save original address: %" PRIxPTR, (uintptr_t) prev_func);
    } else {
        LOG("hook_tag >>>>> hooked. stub: %" PRIxPTR", status: %d, caller_path_name: %s, sym_name: %s, new_func: %" PRIxPTR", prev_func: %" PRIxPTR", arg: %" PRIxPTR,
            (uintptr_t) task_stub, status_code, caller_path_name, sym_name, (uintptr_t) new_func,
            (uintptr_t) prev_func, (uintptr_t) arg);
    }
}

char *jstringToChars(JNIEnv *env, jstring jstr) {
    if (jstr == nullptr) {
        return nullptr;
    }

    jboolean isCopy = JNI_FALSE;
    const char *str = env->GetStringUTFChars(jstr, &isCopy);
    char *ret = strdup(str);
    env->ReleaseStringUTFChars(jstr, str);
    return ret;
}

void printJavaStack() {
    JNIEnv *jniEnv = nullptr;
    // JNIEnv 是绑定线程的，所以这里要重新取
    kJvm->GetEnv((void **) &jniEnv, JNI_VERSION_1_6);
    auto java_stack = reinterpret_cast<jstring>(jniEnv->CallStaticObjectMethod(kJavaClass,
                                                                             kMethodGetStack));
    if (nullptr == java_stack) {
        return;
    }
    char *stack = jstringToChars(jniEnv, java_stack);
    LOG(">>>>> stack:\n%s", stack);
    free(stack);

    jniEnv->DeleteLocalRef(java_stack);
}

static int myPthread_create_proxy(pthread_t *thread, const pthread_attr_t *attr,
                                  void *(*start_routine)(void *), void *arg) {

    LOG(">>>>> proxy thread create");
    BYTEHOOK_STACK_SCOPE();
    printJavaStack();
    int result = BYTEHOOK_CALL_PREV(myPthread_create_proxy, thread, attr,
                                    *start_routine, arg);
    return result;
}


static int hacker_hook_thread(JNIEnv *env, jclass thiz) {
    (void) env, (void) thiz;
    pthread_p_stub = bytehook_hook_single("libart.so", nullptr, "pthread_create",
                                          (void *) myPthread_create_proxy,
                                          pthread_p_hooked_callback, nullptr);
    return 0;
}

static int hacker_unhook(JNIEnv *env, jclass thiz) {
    (void) env, (void) thiz;

    if (nullptr != pthread_p_stub) {
        bytehook_unhook(pthread_p_stub);
        pthread_p_stub = nullptr;
    }

    return 0;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    (void) reserved;

    if (nullptr == vm) return JNI_ERR;
    kJvm = vm;
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) return JNI_ERR;


    jclass cls;
    if (nullptr == (cls = env->FindClass(HACKER_JNI_CLASS_NAME))) return JNI_ERR;
    kJavaClass = reinterpret_cast<jclass>(env->NewGlobalRef(cls));
    kMethodGetStack = env->GetStaticMethodID(kJavaClass, "getStack", "()Ljava/lang/String;");
    JNINativeMethod m[] = {
            {"enableThreadHookNative", "()I", (void *) hacker_hook_thread},
            {"nativeUnhook",           "()I", (void *) hacker_unhook}
    };
    if (0 != env->RegisterNatives(cls, m, sizeof(m) / sizeof(m[0]))) return JNI_ERR;


    return JNI_VERSION_1_6;
}
