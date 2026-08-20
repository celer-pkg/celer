// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include "nativedispatch.h"
#include <QDebug>
#include <QJniEnvironment>

jobject JNICALL

invokeNativeInvocationHandler(JNIEnv* env, jobject thiz,
                              jlong nativeHandle, jstring method, jobjectArray args)
{
    NativeInterface* context = reinterpret_cast<NativeInterface*>(nativeHandle);
    if (!context) {
        qDebug() << "Evil stuff in native invocation handler!!";
        return nullptr;
    }
    return context->qt_invoke(method, args);
}

void registerNativeInvocationHandler() {
    QJniEnvironment env;

    jclass clazz = env.findClass("org/qtproject/qtjennydemo/NativeInvocationHandler");
    if (!clazz) {
        qWarning("Failed to find class NativeInvocationHandler");
        return;
    }

    JNINativeMethod method = {
        "invokeNative", // Java method name
        "(JLjava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;", // JNI signature
        reinterpret_cast<void*>(invokeNativeInvocationHandler) // Pointer to native function
    };

    if (!env.registerNativeMethods(clazz, {method})) {
        qWarning("Failed to register native methods");
        return;
    }
}
