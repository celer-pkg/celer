// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include "sensormanager.h"
#include <QtCore/qcoreapplication.h>
#include <QtCore/qjnitypes.h>

#include <dlfcn.h>

Q_DECLARE_JNI_CLASS(AndroidContext, "android/content/Context")
Q_DECLARE_JNI_CLASS(Sensor, "android/hardware/Sensor")

SensorManager::SensorManager()
{
    auto sensorService =
            QJniObject::getStaticField<QtJniTypes::AndroidContext, jstring>("SENSOR_SERVICE");

    QJniObject context = QNativeInterface::QAndroidApplication::context();
    m_sensorManager = context.callMethod<jobject>("getSystemService",
                                                  sensorService.object<jstring>());
    setObjectName("QtSensorsLooperThread");
    start();
    m_waitForStart.acquire();
}

SensorManager::~SensorManager()
{
    m_quit.storeRelaxed(1);
    wait();
}

QJniObject SensorManager::javaSensor(const ASensor *sensor) const
{
    return m_sensorManager.callMethod<QtJniTypes::Sensor>("getDefaultSensor",
                                                          ASensor_getType(sensor));
}

QSharedPointer<SensorManager> &SensorManager::instance()
{
    static QSharedPointer<SensorManager> looper{new SensorManager};
    return looper;
}

ALooper *SensorManager::looper() const
{
    return m_looper;
}

static inline ASensorManager* androidManager()
{
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    auto packageName = context.callMethod<jstring>("getPackageName").toString().toUtf8();

#if __ANDROID_API__ >= 26
    return ASensorManager_getInstanceForPackage(packageName.constData());
#else
    if (QNativeInterface::QAndroidApplication::sdkVersion() >= 26) {
        using GetInstanceForPackage = ASensorManager *(*)(const char *);
        auto handler = dlopen("libandroid.so", RTLD_NOW);
        auto function = GetInstanceForPackage(dlsym(handler, "ASensorManager_getInstanceForPackage"));
        if (function) {
            auto res = function(packageName.constData());
            dlclose(handler);
            return res;
        }
        dlclose(handler);
    }
    return ASensorManager_getInstance();
#endif
}
ASensorManager *SensorManager::manager() const
{
    static auto sensorManger = androidManager();
    return sensorManger;
}

QString SensorManager::description(const ASensor *sensor) const
{
    return QString::fromUtf8(ASensor_getName(sensor)) + " " + ASensor_getVendor(sensor)
            + " v" + QString::number(javaSensor(sensor).callMethod<jint>("getVersion"));
}

double SensorManager::getMaximumRange(const ASensor *sensor) const
{
    return qreal(javaSensor(sensor).callMethod<jfloat>("getMaximumRange"));
}

void SensorManager::run()
{
    m_looper = ALooper_prepare(0);
    m_waitForStart.release();
    do {
        // Result can be also ALOOPER_POLL_CALLBACK or identifier, those are ignored.
        int result = ALooper_pollOnce(5 /*ms*/, nullptr, nullptr, nullptr);
        if (result == ALOOPER_POLL_TIMEOUT || result == ALOOPER_POLL_WAKE) {
            QThread::yieldCurrentThread();
        } else if (result == ALOOPER_POLL_ERROR) {
            qCritical("SensorManager::run() ALooper_pollOnce() returned ALOOPER_POLL_ERROR");
            break;
        }
    } while (!m_quit.loadRelaxed());
}
