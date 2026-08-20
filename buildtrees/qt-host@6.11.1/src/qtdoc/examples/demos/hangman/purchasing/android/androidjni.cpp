// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/qdatetime.h>
#include <jni.h>
#include <QJniObject>
#include <QTimeZone>

#include "androidinapppurchasebackend.h"

QT_USE_NAMESPACE

static void purchasedProductsQueried(JNIEnv *, jclass, jlong nativePointer)
{
    AndroidInAppPurchaseBackend *backend = reinterpret_cast<AndroidInAppPurchaseBackend *>(nativePointer);
    QMetaObject::invokeMethod(backend,
                              "registerReady",
                              Qt::AutoConnection);
}

static void registerProduct(JNIEnv *, jclass, jlong nativePointer, jstring productId, jstring price, jstring title, jstring description)
{
    AndroidInAppPurchaseBackend *backend = reinterpret_cast<AndroidInAppPurchaseBackend *>(nativePointer);
    QMetaObject::invokeMethod(backend,
                              "registerProduct",
                              Qt::AutoConnection,
                              Q_ARG(QString, QJniObject(productId).toString()),
                              Q_ARG(QString, QJniObject(price).toString()),
                              Q_ARG(QString, QJniObject(title).toString()),
                              Q_ARG(QString, QJniObject(description).toString()));
}

static void registerPurchased(JNIEnv *, jclass, jlong nativePointer, jstring identifier,
                              jstring signature, jstring data, jstring purchaseToken, jstring orderId, jlong timestamp)
{
    QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(qint64(timestamp));
    dateTime.setTimeZone(QTimeZone::LocalTime);

    AndroidInAppPurchaseBackend *backend = reinterpret_cast<AndroidInAppPurchaseBackend *>(nativePointer);
    QMetaObject::invokeMethod(backend,
                              "registerPurchased",
                              Qt::AutoConnection,
                              Q_ARG(QString, QJniObject(identifier).toString()),
                              Q_ARG(QString, QJniObject(signature).toString()),
                              Q_ARG(QString, QJniObject(data).toString()),
                              Q_ARG(QString, QJniObject(purchaseToken).toString()),
                              Q_ARG(QString, QJniObject(orderId).toString()),
                              Q_ARG(QDateTime, dateTime));
}

static void purchaseSucceeded(JNIEnv *, jclass, jlong nativePointer, jint requestCode,
                              jstring signature, jstring data, jstring purchaseToken, jstring orderId, jlong timestamp)
{
    QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(qint64(timestamp));
    dateTime.setTimeZone(QTimeZone::LocalTime);

    AndroidInAppPurchaseBackend *backend = reinterpret_cast<AndroidInAppPurchaseBackend *>(nativePointer);
    QMetaObject::invokeMethod(backend,
                              "purchaseSucceeded",
                              Qt::AutoConnection,
                              Q_ARG(int, int(requestCode)),
                              Q_ARG(QString, QJniObject(signature).toString()),
                              Q_ARG(QString, QJniObject(data).toString()),
                              Q_ARG(QString, QJniObject(purchaseToken).toString()),
                              Q_ARG(QString, QJniObject(orderId).toString()),
                              Q_ARG(QDateTime, dateTime));
}

static void purchaseFailed(JNIEnv *, jclass, jlong nativePointer, jint requestCode, jint failureReason, jstring errorString)
{
    AndroidInAppPurchaseBackend *backend = reinterpret_cast<AndroidInAppPurchaseBackend *>(nativePointer);
    QMetaObject::invokeMethod(backend,
                              "purchaseFailed",
                              Qt::AutoConnection,
                              Q_ARG(int, int(requestCode)),
                              Q_ARG(int, int(failureReason)),
                              Q_ARG(QString, QJniObject(errorString).toString()));
}

static JNINativeMethod methods[] = {
    {"purchasedProductsQueried", "(J)V", (void *)purchasedProductsQueried},
    {"registerProduct", "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", (void *)registerProduct},
    {"registerPurchased", "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;J)V", (void *)registerPurchased},
    {"purchaseSucceeded", "(JILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;J)V", (void *)purchaseSucceeded},
    {"purchaseFailed", "(JIILjava/lang/String;)V", (void *)purchaseFailed}
};

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *)
{
    static bool initialized = false;
    if (initialized){
        return JNI_VERSION_1_6;}
    initialized = true;

    JNIEnv *env;
    if (vm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK){
        return JNI_ERR;}

    jclass clazz = env->FindClass("org/qtproject/qt/android/purchasing/InAppPurchase");
    if (!clazz){
        return JNI_ERR;}

    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) < 0){
        return JNI_ERR;}

    return JNI_VERSION_1_6;
}
