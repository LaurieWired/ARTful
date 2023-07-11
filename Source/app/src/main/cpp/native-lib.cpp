#include <jni.h>
#include <string>
#include <android/log.h>
#include <sys/mman.h>
#include <unistd.h>

void* hookArtMethod(JNIEnv* env, char* className, char* methodName, char* methodSignature);
void* hookArtMethod(JNIEnv* env, jobject targetObject);
void overwriteArtStructureInMemory(void* targetMethod, void* newMethod);

extern "C" JNIEXPORT jstring JNICALL
Java_com_app_artful_MainActivity_replaceMethod(
        JNIEnv* env,
        jobject) {
    std::string hello = "Hello from C++";

    // Manipulation target
    void* maliciousArtMethod;
    void* benignArtMethod;

    benignArtMethod = hookArtMethod(env, "com/app/artful/MainActivity", "benignMethod", "()Ljava/lang/String;");
    maliciousArtMethod = hookArtMethod(env, "com/app/artful/MainActivity", "maliciousMethod", "()Ljava/lang/String;");

    __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Done");

    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_app_artful_MainActivity_swapBuildGetRadioVersion(JNIEnv* env, jobject /* this */, jobject targetObject) {
    void* getRadioVersionArtMethod = hookArtMethod(env, "android/os/Build", "getRadioVersion", "()Ljava/lang/String;");
    void* newArtMethod = hookArtMethod(env, targetObject);

    overwriteArtStructureInMemory(getRadioVersionArtMethod, newArtMethod);
}

void overwriteArtStructureInMemory(void* targetMethod, void* newMethod) {
    // FIXME remove hardcoded bytes to account for architecture
    memcpy(targetMethod, newMethod, 32);
}

// Overload method with signature option

/*
 * Hooks the ArtMethod object for a specified static method.
 * Note: Method MUST be static.
 * Args: Java Native Interface pointer and target Java object
 */
void* hookArtMethod(JNIEnv* env, jobject targetObject) {
    void* hookedArtMethod = NULL;

    // Find the runtime ArtMethod object
    jclass executableClass = env->FindClass("java/lang/reflect/Executable");
    if (executableClass == NULL) {
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Class java/lang/reflect/Executable not found");
        return NULL;
    }
    jfieldID artMethodID = env->GetFieldID(executableClass, "artMethod", "J");
    if (artMethodID == NULL) {
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Field artMethod not found");
        return NULL;
    }

    // Return the artMethod field from the associated method as a pointer
    hookedArtMethod = (void*) env->GetLongField(targetObject, artMethodID);

    return hookedArtMethod;
}

/*
 * Hooks the ArtMethod object for a specified static method.
 * Note: Method MUST be static.
 * Args: Java Native Interface pointer and target Java method attributes
 * Example call: hookArtMethod(env, "com/app/artful/MainActivity", "benignMethod", "()Ljava/lang/String;");
 */
void* hookArtMethod(JNIEnv* env, char* className, char* methodName, char* methodSignature) {
    void* hookedArtMethod = NULL;

    // Hook the target class via the JNI
    jclass targetClass = env->FindClass(className);
    if (targetClass == NULL) {
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Target class %s not found", className);
        return NULL;
    }

    // Make sure the method was static FIXME
    jmethodID targetMethodID = env->GetStaticMethodID(targetClass, methodName, methodSignature);
    jobject targetMethod = env->ToReflectedMethod(targetClass, targetMethodID, JNI_FALSE);

    // Find the runtime ArtMethod object
    jclass executableClass = env->FindClass("java/lang/reflect/Executable");
    if (executableClass == NULL) {
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Class java/lang/reflect/Executable not found");
        return NULL;
    }
    jfieldID artMethodID = env->GetFieldID(executableClass, "artMethod", "J");
    if (artMethodID == NULL) {
        __android_log_print(ANDROID_LOG_DEBUG, "ARTful", "Field artMethod not found");
        return NULL;
    }

    // Return the artMethod field from the associated method as a pointer
    hookedArtMethod = (void*) env->GetLongField(targetMethod, artMethodID);

    return hookedArtMethod;
}
