package com.app.artful;

public class ArtfulLibraryHelper {

    public static void registerNativeMethodsForClass(Class<?> clazz) {
        nativeRegisterNatives(clazz);
    }

    private static native void nativeRegisterNatives(Class<?> clazz);
}
