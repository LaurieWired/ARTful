#include <jni.h>
#include <string>
#include <android/log.h>
#include <sys/mman.h>
#include <unistd.h>

#define DECLARE_RUNTIME_DEBUG_FLAG(x) static constexpr bool x = false;

#define EXECUTABLE_CLASS_PATH "java/lang/reflect/Executable"
#define ART_METHOD_NAME "artMethod"
#define ART_METHOD_SIGNATURE "J"

#define BUILD_CLASS_PATH "android/os/Build"
#define GET_RADIO_VERSION_METHOD_NAME "getRadioVersion"
#define GET_RADIO_VERSION_SIGNATURE "()Ljava/lang/String;"

#define LOG_CLASS_PATH "android/util/Log"
#define LOG_E_METHOD_NAME "e"
#define LOG_E_SIGNATURE "(Ljava/lang/String;Ljava/lang/String;)I"

#define TOAST_CLASS_PATH "android/widget/Toast"
#define MAKE_TEXT_METHOD_NAME "makeText"
#define MAKE_TEXT_SIGNATURE "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;"

#define PATTERN_CLASS_PATH "java/util/regex/Pattern"
#define MATCHES_METHOD_NAME "matches"
#define MATCHES_SIGNATURE "(Ljava/lang/String;Ljava/lang/CharSequence;)Z"

#if defined(__aarch64__) || defined(__x86_64__)
    // 64-bit architecture
    const int NUM_BYTES_TO_OVERWRITE = 32;
#else
    // 32-bit architecture
    const int NUM_BYTES_TO_OVERWRITE = 24;
#endif


/*
 * Dummy ArtMethod values for printing runtime offsets
 */
namespace mirror {
    class Array;
    class Class;
    class ClassLoader;
    class DexCache;
    class IfTable;
    class Object;
    template <typename MirrorType> class ObjectArray;
    class PointerArray;
    class String;
}  // namespace mirror

namespace mirror {

    // Standard compressed reference used in the runtime. Used for StackReference and GC roots.
    template<class MirrorType>
    class CompressedReference  {

    };

}

template<class MirrorType>
class GcRoot {

private:
    // Root visitors take pointers to root_ and place them in CompressedReference** arrays. We use a
    // CompressedReference<mirror::Object> here since it violates strict aliasing requirements to
    // cast CompressedReference<MirrorType>* to CompressedReference<mirror::Object>*.
    mutable mirror::CompressedReference<mirror::Object> root_;

    template <size_t kBufferSize> friend class BufferedRootVisitor;
};


class ArtMethod final {
public:
    // Should the class state be checked on sensitive operations?
    DECLARE_RUNTIME_DEBUG_FLAG(kCheckDeclaringClassState);

    // The runtime dex_method_index is kDexNoIndex. To lower dependencies, we use this
    // constexpr, and ensure that the value is correct in art_method.cc.
    static constexpr uint32_t kRuntimeMethodDexMethodIndex = 0xFFFFFFFF;

    static void printOffsets() {
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Offset of declaring_class_: %zu", offsetof(ArtMethod, declaring_class_));
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Offset of access_flags_: %zu", offsetof(ArtMethod, access_flags_));
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Offset of dex_method_index_: %zu", offsetof(ArtMethod, dex_method_index_));
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Offset of method_index_: %zu", offsetof(ArtMethod, method_index_));
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Offset of hotness_count_: %zu", offsetof(ArtMethod, hotness_count_));
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Offset of imt_index_: %zu", offsetof(ArtMethod, imt_index_));
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Offset of ptr_sized_fields_: %zu", offsetof(ArtMethod, ptr_sized_fields_));
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Offset of ptr_sized_fields_.data_: %zu", offsetof(ArtMethod::PtrSizedFields, data_));
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Offset of ptr_sized_fields_.entry_point_from_quick_compiled_code_: %zu", offsetof(ArtMethod::PtrSizedFields, entry_point_from_quick_compiled_code_));

    }

protected:
    // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
    // The class we are a part of.
    GcRoot<mirror::Class> declaring_class_;

    // Access flags; low 16 bits are defined by spec.
    // Getting and setting this flag needs to be atomic when concurrency is
    // possible, e.g. after this method's class is linked. Such as when setting
    // verifier flags and single-implementation flag.
    std::atomic<std::uint32_t> access_flags_;

    /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

    // Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;

    /* End of dex file fields. */

    // Entry within a dispatch table for this method. For static/direct methods the index is into
    // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
    // ifTable.
    uint16_t method_index_;

    union {
        // Non-abstract methods: The hotness we measure for this method. Not atomic,
        // as we allow missing increments: if the method is hot, we will see it eventually.
        uint16_t hotness_count_;
        // Abstract methods: IMT index.
        uint16_t imt_index_;
    };

    // Fake padding field gets inserted here.

    // Must be the last fields in the method.
    struct PtrSizedFields {
        // Depending on the method type, the data is
        //   - native method: pointer to the JNI function registered to this method
        //                    or a function to resolve the JNI function,
        //   - resolution method: pointer to a function to resolve the method and
        //                        the JNI function for @CriticalNative.
        //   - conflict method: ImtConflictTable,
        //   - abstract/interface method: the single-implementation if any,
        //   - proxy method: the original interface method or constructor,
        //   - other methods: during AOT the code item offset, at runtime a pointer
        //                    to the code item.
        void* data_;

        // Method dispatch from quick compiled code invokes this pointer which may cause bridging into
        // the interpreter.
        void* entry_point_from_quick_compiled_code_;
    } ptr_sized_fields_;

};


/*
 * Internal methods section
 */

/*
 * Completely overwrites the targetMethod ArtMethod structure in memory. Any
 *  future calls to targetMethod will trigger newMethod code
 */
void overwriteArtStructureInMemory(void* targetMethod, void* newMethod) {
    if (targetMethod != nullptr && newMethod != nullptr) {
        memcpy(targetMethod, newMethod, NUM_BYTES_TO_OVERWRITE);
    } else {
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Unable to overwrite target method");
    }
}

/*
 * Hooks the ArtMethod object for a specified static method.
 *  Overloaded to take in object instead of signature.
 * Note: Method MUST be static.
 * Args: Java Native Interface pointer and target Java object
 */
void* hookArtMethod(JNIEnv* env, jobject targetObject) {
    void* hookedArtMethod = nullptr;

    // Find the runtime ArtMethod object
    jclass executableClass = env->FindClass(EXECUTABLE_CLASS_PATH);
    if (executableClass == nullptr) {
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Class java/lang/reflect/Executable not found");
        return nullptr;
    }
    jfieldID artMethodID = env->GetFieldID(executableClass, ART_METHOD_NAME, ART_METHOD_SIGNATURE);
    if (artMethodID == nullptr) {
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Field artMethod not found");
        return nullptr;
    }

    // Return the artMethod field from the associated method as a pointer
    hookedArtMethod = (void*) env->GetLongField(targetObject, artMethodID);

    return hookedArtMethod;
}

/*
 * Hooks the ArtMethod object for a specified static method.
 *  Overloaded to take in fully-qualified class name, method name, and method signature
 * Note: Method MUST be static.
 * Args: Java Native Interface pointer and target Java method attributes
 * Example call: hookArtMethod(env, "com/app/artful/MainActivity", "benignMethod", "()Ljava/lang/String;");
 */
void* hookArtMethod(JNIEnv* env, const char* className, const char* methodName, const char* methodSignature) {
    void* hookedArtMethod = nullptr;

    // Hook the target class via the JNI
    jclass targetClass = env->FindClass(className);
    if (targetClass == nullptr) {
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Target class %s not found", className);
        return nullptr;
    }

    jmethodID targetMethodID = env->GetStaticMethodID(targetClass, methodName, methodSignature);
    jobject targetMethod = env->ToReflectedMethod(targetClass, targetMethodID, JNI_FALSE);

    // Find the runtime ArtMethod object
    jclass executableClass = env->FindClass(EXECUTABLE_CLASS_PATH);
    if (executableClass == nullptr) {
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Class java/lang/reflect/Executable not found");
        return nullptr;
    }
    jfieldID artMethodID = env->GetFieldID(executableClass, ART_METHOD_NAME, ART_METHOD_SIGNATURE);
    if (artMethodID == nullptr) {
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Field artMethod not found");
        return nullptr;
    }

    // Return the artMethod field from the associated method as a pointer
    hookedArtMethod = (void*) env->GetLongField(targetMethod, artMethodID);

    return hookedArtMethod;
}


/*
 * External methods section. Callable from Java and to be part of ARTful library.
 */

/*
 * Swaps two methods from the user's app by Java object
 */
extern "C" JNIEXPORT void JNICALL
Java_com_app_artful_MainActivity_replaceAppMethodByObject(JNIEnv* env, jobject /* this */, jobject targetMethod, jobject newMethod) {
    void* targetArtMethod;
    void* newArtMethod;

    targetArtMethod = hookArtMethod(env, targetMethod);
    newArtMethod = hookArtMethod(env, newMethod);

    if (targetArtMethod != nullptr && newArtMethod != nullptr) {
        overwriteArtStructureInMemory(targetArtMethod, newArtMethod);
        __android_log_print(ANDROID_LOG_INFO, "ARTful", "Replaced target method");
    } else {
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Failed to replace target method");
    }
}

/*
 * Swaps two methods from the user's app by Java object.
 * Note: Signature is the same since it must match for swapping to work.
 */
extern "C" JNIEXPORT void JNICALL
Java_com_app_artful_MainActivity_replaceAppMethodBySignature(JNIEnv* env, jobject /* this */,
    jstring targetClassName, jstring targetMethodName, jstring newClassName, jstring newMethodName, jstring methodSignature) {

    void* targetArtMethod;
    void* newArtMethod;

    targetArtMethod = hookArtMethod(env, env->GetStringUTFChars(targetClassName, JNI_FALSE),
                                    env->GetStringUTFChars(targetMethodName, JNI_FALSE),
                                    env->GetStringUTFChars(methodSignature, JNI_FALSE));
    newArtMethod = hookArtMethod(env, env->GetStringUTFChars(newClassName, JNI_FALSE),
                                 env->GetStringUTFChars(newMethodName, JNI_FALSE),
                                 env->GetStringUTFChars(methodSignature, JNI_FALSE));

    if (targetArtMethod != nullptr && newArtMethod != nullptr) {
        overwriteArtStructureInMemory(targetArtMethod, newArtMethod);
        __android_log_print(ANDROID_LOG_INFO, "ARTful", "Replaced target method");
    } else {
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Failed to replace target method");
    }
}

/*
 * Prints offsets withing the native runtime structure of ArtMethod
 */
extern "C" JNIEXPORT void JNICALL
Java_com_app_artful_MainActivity_printArtMethodOffsets(JNIEnv* env, jobject /* this */) {
    ArtMethod artMethod;
    artMethod.printOffsets();
}

/*
 * Swaps the Build.getRadioVersion() Android Framework method with a new method.
 * Reference: https://developer.android.com/reference/android/os/Build#getRadioVersion()
 */
extern "C" JNIEXPORT void JNICALL
Java_com_app_artful_MainActivity_replaceGetRadioVersionByObject(JNIEnv* env, jobject /* this */, jobject newObject) {
    void* getRadioVersionArtMethod = hookArtMethod(env, BUILD_CLASS_PATH,
                                                   GET_RADIO_VERSION_METHOD_NAME,
                                                   GET_RADIO_VERSION_SIGNATURE);
    void* newArtMethod = hookArtMethod(env, newObject);

    overwriteArtStructureInMemory(getRadioVersionArtMethod, newArtMethod);
}

extern "C" JNIEXPORT void JNICALL
Java_com_app_artful_MainActivity_replaceGetRadioVersionBySignature(JNIEnv* env, jobject /* this */,
        jstring newClassName, jstring newMethodName) {

    void* targetArtMethod = hookArtMethod(env, BUILD_CLASS_PATH,
                                                   GET_RADIO_VERSION_METHOD_NAME,
                                                   GET_RADIO_VERSION_SIGNATURE);

    void* newArtMethod = hookArtMethod(env, env->GetStringUTFChars(newClassName, JNI_FALSE),
                                 env->GetStringUTFChars(newMethodName, JNI_FALSE),
                                 GET_RADIO_VERSION_SIGNATURE);

    overwriteArtStructureInMemory(targetArtMethod, newArtMethod);
}

/*
 * Swaps the Build.getRadioVersion() Android Framework method with a new method.
 * Reference: https://developer.android.com/reference/android/util/Log#e(java.lang.String,%20java.lang.String)
 */
extern "C" JNIEXPORT void JNICALL
Java_com_app_artful_MainActivity_replaceLogEByObject(JNIEnv* env, jobject /* this */, jobject newObject) {
    void* targetArtMethod = hookArtMethod(env, LOG_CLASS_PATH,
                                                   LOG_E_METHOD_NAME,
                                                   LOG_E_SIGNATURE);
    void* newArtMethod = hookArtMethod(env, newObject);

    overwriteArtStructureInMemory(targetArtMethod, newArtMethod);
}

extern "C" JNIEXPORT void JNICALL
Java_com_app_artful_MainActivity_replaceLogEBySignature(JNIEnv* env, jobject /* this */,
                                                                   jstring newClassName, jstring newMethodName) {

    void* targetArtMethod = hookArtMethod(env, LOG_CLASS_PATH,
                                                   LOG_E_METHOD_NAME,
                                                   LOG_E_SIGNATURE);

    void* newArtMethod = hookArtMethod(env, env->GetStringUTFChars(newClassName, JNI_FALSE),
                                       env->GetStringUTFChars(newMethodName, JNI_FALSE),
                                       LOG_E_SIGNATURE);

    overwriteArtStructureInMemory(targetArtMethod, newArtMethod);
}

/*
 * Swaps the Toast.makeText() Android Framework method with a new method.
 * Reference: https://developer.android.com/reference/android/widget/Toast#makeText(android.content.Context,%20java.lang.CharSequence,%20int)
 */
extern "C" JNIEXPORT void JNICALL
Java_com_app_artful_MainActivity_replaceToastMakeTextByObject(JNIEnv* env, jobject /* this */, jobject newObject) {
    void* targetArtMethod = hookArtMethod(env, TOAST_CLASS_PATH,
                                                   MAKE_TEXT_METHOD_NAME,
                                                   MAKE_TEXT_SIGNATURE);
    void* newArtMethod = hookArtMethod(env, newObject);

    overwriteArtStructureInMemory(targetArtMethod, newArtMethod);
}

extern "C" JNIEXPORT void JNICALL
Java_com_app_artful_MainActivity_replaceToastMakeTextBySignature(JNIEnv* env, jobject /* this */,
                                                        jstring newClassName, jstring newMethodName) {

    void* targetArtMethod = hookArtMethod(env, TOAST_CLASS_PATH,
                                          MAKE_TEXT_METHOD_NAME,
                                          MAKE_TEXT_SIGNATURE);

    void* newArtMethod = hookArtMethod(env, env->GetStringUTFChars(newClassName, JNI_FALSE),
                                       env->GetStringUTFChars(newMethodName, JNI_FALSE),
                                       MAKE_TEXT_SIGNATURE);

    overwriteArtStructureInMemory(targetArtMethod, newArtMethod);
}

/*
 * Swaps the Pattern.matches() Android Framework method with a new method.
 * Reference: https://developer.android.com/reference/java/util/regex/Pattern#matches(java.lang.String,%20java.lang.CharSequence)
 */
extern "C" JNIEXPORT void JNICALL
Java_com_app_artful_MainActivity_replacePatternMatchesByObject(JNIEnv* env, jobject /* this */, jobject newObject) {
    void* targetArtMethod = hookArtMethod(env, PATTERN_CLASS_PATH,
                                          MATCHES_METHOD_NAME,
                                          MATCHES_SIGNATURE);
    void* newArtMethod = hookArtMethod(env, newObject);

    overwriteArtStructureInMemory(targetArtMethod, newArtMethod);
}

extern "C" JNIEXPORT void JNICALL
Java_com_app_artful_MainActivity_replacePatternMatchesBySignature(JNIEnv* env, jobject /* this */,
                                                                 jstring newClassName, jstring newMethodName) {

    void* targetArtMethod = hookArtMethod(env, PATTERN_CLASS_PATH,
                                          MATCHES_METHOD_NAME,
                                          MATCHES_SIGNATURE);

    void* newArtMethod = hookArtMethod(env, env->GetStringUTFChars(newClassName, JNI_FALSE),
                                       env->GetStringUTFChars(newMethodName, JNI_FALSE),
                                       MATCHES_SIGNATURE);

    overwriteArtStructureInMemory(targetArtMethod, newArtMethod);
}
