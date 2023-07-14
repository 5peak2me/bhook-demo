#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <jni.h>
#include <sys/time.h>
#include <android/log.h>
#include <sys/mman.h>
#include <stdio.h>
#include <ctype.h>
#include "bytehook.h"

#define HACKER_JNI_VERSION    JNI_VERSION_1_6
#define HACKER_JNI_CLASS_NAME "com/l3gacy/app/bhook/NativeHacker"
#define HACKER_TAG            "hook_tag"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#define LOG(fmt, ...)  __android_log_print(ANDROID_LOG_INFO, HACKER_TAG, fmt, ##__VA_ARGS__)
#pragma clang diagnostic pop

typedef struct Parcel Parcel;
typedef struct hidl_string hidl_string;
typedef struct IPCThreadState IPCThreadState;

typedef int (*transact_t)(IPCThreadState *, unsigned int, Parcel *, Parcel *, unsigned int);


#define OPEN_DEF(fn) \
static fn##_t fn##_prev = NULL; \
static bytehook_stub_t fn##_stub = NULL; \
static void fn##_hooked_callback(bytehook_stub_t task_stub, int status_code, const char *caller_path_name, const char *sym_name, void *new_func, void *prev_func, void *arg) \
{ \
    if(BYTEHOOK_STATUS_CODE_ORIG_ADDR == status_code) \
    { \
        fn##_prev = (fn##_t)prev_func; \
        LOG(">>>>> save original address: %" PRIxPTR, (uintptr_t)prev_func); \
    } \
    else \
    { \
        LOG(">>>>> hooked. stub: %" PRIxPTR", status: %d, caller_path_name: %s, sym_name: %s, new_func: %" PRIxPTR", prev_func: %" PRIxPTR", arg: %" PRIxPTR, \
            (uintptr_t)task_stub, status_code, caller_path_name, sym_name, (uintptr_t)new_func, (uintptr_t)prev_func, (uintptr_t)arg); \
    } \
}

OPEN_DEF(transact)


static void debug(const char *sym, const char *pathname, int flags, int fd, void *lr) {
    Dl_info info;
    memset(&info, 0, sizeof(info));
    dladdr(lr, &info);

    LOG("proxy %s(\"%s\", %d), return FD: %d, called from: %s (%s)",
        sym, pathname, flags, fd, info.dli_fname, info.dli_sname);
}

static void HexDump(char *str, int size) {
    char out[128 * 2 + 50];
//    char outstr[128*2+50];
//    char line[17];
//    char* pc;
    int ix;
    int jx;
    //并不会因为unsigned long而读取8个字节，还是只读了1个字节
    LOG("bytehook address %ld", (unsigned long) *(str + 0x28));
    for (ix = 0; ix < size; ix += 16) {
        printf("%.8xH:", ix);
        //LOG("%.8xH:", ix);//打印十六进制
        for (jx = 0; jx != 16; jx++) {
            if (ix + jx >= size) {
                //printf(" ");
                //LOG(" ");
                //sprintf(out+(ix+jx)*2," ");
            } else {
                sprintf(out + (ix + jx) * 2, "%02X", (unsigned char) *(str + ix + jx));
                //printf("%.2X ",(unsigned char)*(str+ix+jx));
                //LOG("%.2X ",(unsigned char)*(str+ix+jx));
            }
        }
//打印字符串
        /*{
            memcpy(line, (const void *) (str + ix), 16);
            pc=line;
            while (pc!=line+16)
            {
                if ((*pc=='\n') || (*pc=='\t'))
                {
                    *pc=' ';
                }
                else if ((unsigned char)*pc < 0x10)
                {
                    *pc='.';
                }
                pc++;
            }
            line[16]='\n';
            printf(" ; %s\n",line);
            sprintf(outstr+ix*16,"%s",line);
            //LOG(" ; %s\n",line);
        }*/
    }
    LOG("bytehook out %s", out);
    //LOG("bytehook outstr %s", outstr);
}

static int transact_proxy_auto(struct IPCThreadState *f, unsigned int a, Parcel *b, Parcel *c,
                               unsigned int d) {
    LOG("ssssssss");
    int result;
    /**
     *          data in parcel
        const mData_LOC         = 0x28;
        const mDataSize_LOC     = 0x30;
        const mObjects_LOC      = 0x48;
        const mObjectsSize_LOC  = 0x50;
     */
    unsigned long *mData = (unsigned long *) ((uintptr_t) b + 0x28);
    unsigned int *mDataSize = (unsigned int *) ((uintptr_t) b + 0x30);
    unsigned long *mObject = (unsigned long *) ((uintptr_t) b + 0x48);
    unsigned int *mObjectSize = (unsigned int *) ((uintptr_t) b + 0x50);

    unsigned int *block = (unsigned int *) (*mData +
                                            0x2c); // parcel 中紧跟在 interface_token 之后的 int 数据

    if (a == 11 && *block == 1) {  // transaction_code 为 11, parcel 中对应的 int 值为 1
        LOG("transaction_code is: %d, block is: %d", a, *block);
        const char *target_interface = "vendor.huawei.hardware.ai@1.1::IAiModelMngr";
        if (strcmp((char *) *mData, target_interface) ==
            0) {   // interface_token 为 vendor.huawei.hardware.ai@1.1::IAiModelMngr
            LOG("Params Transaction_code=%d Parcel_data=0x%" PRIxPTR" Parcel_reply=0x%" PRIxPTR" unsigned_int=%d",
                a, (uintptr_t) b, (uintptr_t) c, d);
            LOG("Data  : 0x%" PRIxPTR", Data_Size  : %d", (uintptr_t) *mData, *mDataSize);
            LOG("Object: 0x%" PRIxPTR", Object_Size: %d", (uintptr_t) *mObject, *mObjectSize);
            unsigned long *object_offset = (unsigned long *) (*mObject + 24);
            unsigned long object_pos = *mData + *object_offset;
            LOG("bytehook param object_offset=%lx, object_pos=%lx", *object_offset, object_pos);
            //取该object的buffer区域
            unsigned long *buffer = (unsigned long *) (object_pos + 0x8);

            //取buffer区域中fd以及size
            unsigned int *fd = (unsigned int *) (*buffer + 0x0c);
            unsigned int *size = (unsigned int *) (*buffer + 0x18);
            LOG("bytehook param fd=%x, size=%x", *fd, *size);
            void *ptr = mmap(NULL, *size, PROT_READ | PROT_WRITE, MAP_SHARED, (int) *fd, 0);

            LOG("bytehook target: 0x%" PRIxPTR,
                ((uintptr_t) ptr + 0x53e8));  //  修改 ashmem 中 0x53e8 偏移处的数据
            HexDump((char *) ((uintptr_t) ptr + 0x53e8), 128);
            int *old_v = (int *) ((uintptr_t) ptr + 0x53e8);
            LOG("value change : %x --> %x", *old_v, ~(*old_v));

            *(int *) ((uintptr_t) ptr + 0x53e8) = ~(*old_v);    //  取反 即"FBFEFEFE"
            int ret = munmap(ptr, *size);
            if (ret == 0) {
                LOG("bytehook update successfully!");
            }
        }
    }
    result = BYTEHOOK_CALL_PREV(transact_proxy_auto, transact_t, f, a, b, c, d);
    debug("transact auto", "test", (int) a, result, BYTEHOOK_RETURN_ADDRESS());

    BYTEHOOK_POP_STACK();
    return result;
}


static bool allow_filter(const char *caller_path_name, void *arg) {
    (void) arg;

    if (NULL != strstr(caller_path_name, "libc.so")) return false;
    if (NULL != strstr(caller_path_name, "libbase.so")) return false;
    if (NULL != strstr(caller_path_name, "liblog.so")) return false;
    if (NULL != strstr(caller_path_name, "libunwindstack.so")) return false;
    if (NULL != strstr(caller_path_name, "libutils.so")) return false;
    // ......

    return true;
}

static bool allow_filter_for_hook_all(const char *caller_path_name, void *arg) {
    (void) arg;

    if (NULL != strstr(caller_path_name, "liblog.so")) return false;

    return true;
}

static int hacker_hook(JNIEnv *env, jobject thiz, jint type) {

    (void) env, (void) thiz;

    if (NULL != transact_stub) return -1;
    LOG("transact_stub == NULL");

    void *transact_proxy;

    if (BYTEHOOK_MODE_MANUAL == bytehook_get_mode()) {
        LOG("mode: MANUAL");
        transact_proxy = (void *) transact_proxy_auto;
    } else {
        LOG("mode: AUTOMATIC");
        transact_proxy = (void *) transact_proxy_auto;
    }

    if (0 == type) {
        LOG("hook_single method");
        //add you want to hook
//        transact_stub = bytehook_hook_single("/system/lib64/vndk-sp-29/libhidlbase.so", NULL,
        transact_stub = bytehook_hook_single("/system/lib/libhidlbase.so", NULL,
                                             "_ZN7android8hardware10BpHwBinder8transactEjRKNS0_6ParcelEPS2_jNSt3__18functionIFvRS2_EEE",
                                             transact_proxy, transact_hooked_callback, NULL);
    } else if (1 == type) {
        transact_stub = bytehook_hook_partial(allow_filter, NULL, NULL, "transact", transact_proxy,
                                              transact_hooked_callback, NULL);

    } else if (2 == type) {
        // Here we are not really using bytehook_hook_all().
        //
        // In the sample app, we use logcat to output debugging information, so we need to
        // filter out liblog.so when hook all.
        //
        // Because in some Android versions, liblog.so will call open() when executing
        // __android_log_print(). This is not a problem in itself, but it will indirectly
        // cause the InitLogging() function of libartbase.so to re-enter and cause a deadlock,
        // eventually leading to ANR.
        //
        // In this sample app, don't do this:
        //
        // open_stub = bytehook_hook_all(NULL, "open", open_proxy, open_hooked_callback, NULL);
        // open_real_stub = bytehook_hook_all(NULL, "__open_real", open_real_proxy, open_real_hooked_callback, NULL);
        // open2_stub = bytehook_hook_all(NULL, "__open_2", open2_proxy, open2_hooked_callback, NULL);

        transact_stub = bytehook_hook_partial(allow_filter_for_hook_all, NULL, NULL, "transact",
                                              transact_proxy, transact_hooked_callback, NULL);
    }

    return 0;
}

static int hacker_unhook(JNIEnv *env, jobject thiz) {
    (void) env, (void) thiz;

    if (NULL != transact_stub) {
        bytehook_unhook(transact_stub);
        transact_stub = NULL;
    }
    return 0;
}

static void hacker_dump_records(JNIEnv *env, jobject thiz, jstring pathname) {
    (void) thiz;

    const char *c_pathname = (*env)->GetStringUTFChars(env, pathname, 0);
    if (NULL == c_pathname) return;

    int fd = open(c_pathname, O_CREAT | O_WRONLY | O_CLOEXEC | O_TRUNC | O_APPEND,
                  S_IRUSR | S_IWUSR);
    if (fd >= 0) {
        bytehook_dump_records(fd, BYTEHOOK_RECORD_ITEM_ALL);
        close(fd);
    }

    (*env)->ReleaseStringUTFChars(env, pathname, c_pathname);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {

    (void) reserved;

    if (NULL == vm) return JNI_ERR;

    JNIEnv *env;
    if (JNI_OK != (*vm)->GetEnv(vm, (void **) &env, HACKER_JNI_VERSION)) return JNI_ERR;
    if (NULL == env || NULL == *env) return JNI_ERR;

    jclass cls;
    if (NULL == (cls = (*env)->FindClass(env, HACKER_JNI_CLASS_NAME))) return JNI_ERR;

    JNINativeMethod m[] = {
            {"nativeHook",        "(I)I",                  (void *) hacker_hook},
            {"nativeUnhook",      "()I",                   (void *) hacker_unhook},
            {"nativeDumpRecords", "(Ljava/lang/String;)V", (void *) hacker_dump_records}
    };
    LOG("ready to registerNatives");
    if (0 != (*env)->RegisterNatives(env, cls, m, sizeof(m) / sizeof(m[0]))) return JNI_ERR;

    return HACKER_JNI_VERSION;
}
