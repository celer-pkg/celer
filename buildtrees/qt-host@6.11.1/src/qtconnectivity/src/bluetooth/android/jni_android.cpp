// Copyright (C) 2016 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <jni.h>
#include <android/log.h>
#include <QtCore/QLoggingCategory>
#include <QtBluetooth/qtbluetoothglobal.h>
#include "android/jni_android_p.h"
#include "android/androidbroadcastreceiver_p.h"
#include "android/serveracceptancethread_p.h"
#include "android/inputstreamthread_p.h"
#include "android/lowenergynotificationhub_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

typedef QHash<QByteArray, QString> JCachedStringFields;
Q_GLOBAL_STATIC(JCachedStringFields, cachedStringFields)
Q_GLOBAL_STATIC(QMutex, stringCacheMutex);

/*
 * This function operates on the assumption that each
 * field is of type java/lang/String.
 */
QString valueFromStaticFieldCache(const char *key, const char *className, const char *fieldName)
{
    QMutexLocker lock(stringCacheMutex());
    JCachedStringFields::iterator it = cachedStringFields()->find(key);
    if (it == cachedStringFields()->end()) {
        QJniEnvironment env;
        QJniObject fieldValue = QJniObject::getStaticObjectField(
                                            className, fieldName, "Ljava/lang/String;");
        if (!fieldValue.isValid()) {
            cachedStringFields()->insert(key, {});
            return {};
        }
        const QString string = fieldValue.toString();
        cachedStringFields()->insert(key, string);
        return string;
    } else {
        return it.value();
    }
}

void QtBroadcastReceiver_jniOnReceive(JNIEnv *env, jobject /*javaObject*/,
                                      jlong qtObject, QtJniTypes::Context context,
                                      QtJniTypes::Intent intent)
{
    reinterpret_cast<AndroidBroadcastReceiver*>(qtObject)->onReceive(env, context.object(), intent.object());
}
Q_DECLARE_JNI_NATIVE_METHOD(QtBroadcastReceiver_jniOnReceive, jniOnReceive)

static void QtBluetoothSocketServer_errorOccurred(JNIEnv */*env*/, jobject /*javaObject*/,
                                           jlong qtObject, jint errorCode)
{
    reinterpret_cast<ServerAcceptanceThread*>(qtObject)->javaThreadErrorOccurred(errorCode);
}
Q_DECLARE_JNI_NATIVE_METHOD(QtBluetoothSocketServer_errorOccurred, errorOccurred)

static void QtBluetoothSocketServer_newSocket(JNIEnv */*env*/, jobject /*javaObject*/,
                                              jlong qtObject, QtJniTypes::BluetoothSocket socket)
{
    reinterpret_cast<ServerAcceptanceThread*>(qtObject)->javaNewSocket(socket.object());
}
Q_DECLARE_JNI_NATIVE_METHOD(QtBluetoothSocketServer_newSocket, newSocket)

static void QtBluetoothInputStreamThread_errorOccurred(JNIEnv */*env*/, jobject /*javaObject*/,
                                           jlong qtObject, jint errorCode)
{
    reinterpret_cast<InputStreamThread*>(qtObject)->javaThreadErrorOccurred(errorCode);
}
Q_DECLARE_JNI_NATIVE_METHOD(QtBluetoothInputStreamThread_errorOccurred, errorOccurred)

static void QtBluetoothInputStreamThread_readyData(JNIEnv */*env*/, jobject /*javaObject*/,
                                       jlong qtObject, jbyteArray buffer, jint bufferLength)
{
    reinterpret_cast<InputStreamThread*>(qtObject)->javaReadyRead(buffer, bufferLength);
}
Q_DECLARE_JNI_NATIVE_METHOD(QtBluetoothInputStreamThread_readyData, readyData)

void QtBluetoothLE_leScanResult(JNIEnv *env, jobject, jlong qtObject,
                                QtJniTypes::BluetoothDevice bluetoothDevice, jint rssi,
                                jbyteArray scanRecord)
{
    if (Q_UNLIKELY(qtObject == 0))
        return;

    reinterpret_cast<AndroidBroadcastReceiver*>(qtObject)->onReceiveLeScan(
                                                                env, bluetoothDevice.object(), rssi,
                                                                scanRecord);
}
Q_DECLARE_JNI_NATIVE_METHOD(QtBluetoothLE_leScanResult, leScanResult)

static const char logTag[] = "QtBluetooth";
static const char classErrorMsg[] = "Can't find class \"%s\"";

#define FIND_AND_CHECK_CLASS(CLASS_NAME)                                     \
clazz = env.findClass<CLASS_NAME>();                                         \
if (!clazz) {                                                                \
    __android_log_print(ANDROID_LOG_FATAL, logTag, classErrorMsg,            \
                        QtJniTypes::Traits<CLASS_NAME>::className().data()); \
    return JNI_FALSE;                                                        \
}                                                                            \

#define LEHUB_SCOPED_METHOD(Method) Q_JNI_NATIVE_SCOPED_METHOD(Method, LowEnergyNotificationHub)

static bool registerNatives()
{
    jclass clazz;
    QJniEnvironment env;

    FIND_AND_CHECK_CLASS(QtJniTypes::QtBtBroadcastReceiver);
    if (!env.registerNativeMethods(clazz,
                                  {
                                      Q_JNI_NATIVE_METHOD(QtBroadcastReceiver_jniOnReceive)
                                  }))
    {
        __android_log_print(ANDROID_LOG_FATAL, logTag,
                            "registerNativeMethods for BroadcastReceiver failed");
        return false;
    }

    FIND_AND_CHECK_CLASS(QtJniTypes::QtBtLECentral);
    if (!env.registerNativeMethods(clazz,
                              {
                                   Q_JNI_NATIVE_METHOD(QtBluetoothLE_leScanResult),
                                   LEHUB_SCOPED_METHOD(lowEnergy_connectionChange),
                                   LEHUB_SCOPED_METHOD(lowEnergy_mtuChanged),
                                   LEHUB_SCOPED_METHOD(lowEnergy_servicesDiscovered),
                                   LEHUB_SCOPED_METHOD(lowEnergy_serviceDetailsDiscovered),
                                   LEHUB_SCOPED_METHOD(lowEnergy_characteristicRead),
                                   LEHUB_SCOPED_METHOD(lowEnergy_descriptorRead),
                                   LEHUB_SCOPED_METHOD(lowEnergy_characteristicWritten),
                                   LEHUB_SCOPED_METHOD(lowEnergy_descriptorWritten),
                                   LEHUB_SCOPED_METHOD(lowEnergy_characteristicChanged),
                                   LEHUB_SCOPED_METHOD(lowEnergy_serviceError),
                                   LEHUB_SCOPED_METHOD(lowEnergy_remoteRssiRead)
                               }))
    {
        __android_log_print(ANDROID_LOG_FATAL, logTag,
                            "registerNativeMethods for QBLuetoothLE failed");
        return false;
    }

    FIND_AND_CHECK_CLASS(QtJniTypes::QtBtLEServer);
    if (!env.registerNativeMethods(clazz,
                            {
                                 LEHUB_SCOPED_METHOD(lowEnergy_connectionChange),
                                 LEHUB_SCOPED_METHOD(lowEnergy_mtuChanged),
                                 LEHUB_SCOPED_METHOD(lowEnergy_advertisementError),
                                 LEHUB_SCOPED_METHOD(lowEnergy_serverCharacteristicChanged),
                                 LEHUB_SCOPED_METHOD(lowEnergy_serverDescriptorWritten)
                             }))
    {
        __android_log_print(ANDROID_LOG_FATAL, logTag,
                            "registerNativeMethods for QBLuetoothLEServer failed");
        return false;
    }

    FIND_AND_CHECK_CLASS(QtJniTypes::QtBtSocketServer);
    if (!env.registerNativeMethods(clazz,
                                  {
                                      Q_JNI_NATIVE_METHOD(QtBluetoothSocketServer_errorOccurred),
                                      Q_JNI_NATIVE_METHOD(QtBluetoothSocketServer_newSocket)
                                  }))
    {
        __android_log_print(ANDROID_LOG_FATAL, logTag,
                            "registerNativeMethods for SocketServer failed");
        return false;
    }

    FIND_AND_CHECK_CLASS(QtJniTypes::QtBtInputStreamThread);
    if (!env.registerNativeMethods(clazz,
                               {
                                   Q_JNI_NATIVE_METHOD(QtBluetoothInputStreamThread_errorOccurred),
                                   Q_JNI_NATIVE_METHOD(QtBluetoothInputStreamThread_readyData)
                               }))
    {
        __android_log_print(ANDROID_LOG_FATAL, logTag,
                            "registerNativeMethods for InputStreamThread failed");
        return false;
    }

    return true;
}

QT_END_NAMESPACE

extern "C" Q_BLUETOOTH_EXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    static bool initialized = false;
    if (initialized)
        return JNI_VERSION_1_6;
    initialized = true;

    typedef union {
        JNIEnv *nativeEnvironment;
        void *venv;
    } UnionJNIEnvToVoid;

    UnionJNIEnvToVoid uenv;
    uenv.venv = 0;

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_6) != JNI_OK) {
        __android_log_print(ANDROID_LOG_FATAL, logTag, "GetEnv failed");
        return -1;
    }

    const auto context = QNativeInterface::QAndroidApplication::context();
    QtJniTypes::QtBtBroadcastReceiver::callStaticMethod<void>("setContext", context);

    if (!registerNatives()) {
        __android_log_print(ANDROID_LOG_FATAL, logTag, "registerNatives failed");
        return -1;
    }

    if (QT_BT_ANDROID().isDebugEnabled())
        __android_log_print(ANDROID_LOG_INFO, logTag, "Bluetooth start");

    return JNI_VERSION_1_6;
}
