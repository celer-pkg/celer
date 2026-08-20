// Copyright (C) 2016 Centria research and development
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qnearfieldmanager_android_p.h"

#include "qndefmessage.h"
#include "qbytearray.h"
#include "qcoreapplication.h"

#include <QtCore/QMetaType>

QT_BEGIN_NAMESPACE

extern "C"
{
    JNIEXPORT void JNICALL Java_org_qtproject_qt_android_nfc_QtNfcBroadcastReceiver_jniOnReceive(
        JNIEnv */*env*/, jobject /*javaObject*/, jlong qtObject, jint state)
    {
        QNearFieldManager::AdapterState adapterState =
                static_cast<QNearFieldManager::AdapterState>(state);
        auto obj = reinterpret_cast<QNearFieldManagerPrivateImpl *>(qtObject);
        Q_ASSERT(obj != nullptr);
        obj->adapterStateChanged(adapterState);
    }
}

Q_GLOBAL_STATIC(QMainNfcNewIntentListener, newIntentListener)

QNearFieldManagerPrivateImpl::QNearFieldManagerPrivateImpl() :
    detecting(false)
{
    qRegisterMetaType<QJniObject>("QJniObject");
    qRegisterMetaType<QNdefMessage>("QNdefMessage");

    broadcastReceiver = QJniObject::construct<QtJniTypes::QtNfcBroadcastReceiver>(
            reinterpret_cast<jlong>(this), QNativeInterface::QAndroidApplication::context());
}

QNearFieldManagerPrivateImpl::~QNearFieldManagerPrivateImpl()
{
    broadcastReceiver.callMethod<void>("unregisterReceiver");
}

void QNearFieldManagerPrivateImpl::onTargetDetected(QNearFieldTargetPrivateImpl *target)
{
    if (target->q_ptr) {
        Q_EMIT targetDetected(target->q_ptr);
        return;
    }

    Q_EMIT targetDetected(new QNearFieldTarget(target, this));
}

void QNearFieldManagerPrivateImpl::onTargetLost(QNearFieldTargetPrivateImpl *target)
{
    Q_EMIT targetLost(target->q_ptr);
}

bool QNearFieldManagerPrivateImpl::isEnabled() const
{
    return QtNfc::isEnabled();
}

bool QNearFieldManagerPrivateImpl::isSupported(QNearFieldTarget::AccessMethod accessMethod) const
{
    if (accessMethod == QNearFieldTarget::UnknownAccess)
        return false;

    return QtNfc::isSupported();
}

bool QNearFieldManagerPrivateImpl::startTargetDetection(QNearFieldTarget::AccessMethod accessMethod)
{
    if (detecting)
        return false;   // Already detecting targets

    if (newIntentListener.isDestroyed())
        return false;

    detecting = true;
    requestedMethod = accessMethod;
    newIntentListener->registerListener(this);
    return true;
}

void QNearFieldManagerPrivateImpl::stopTargetDetection(const QString &)
{
    detecting = false;
    if (newIntentListener.exists())
        newIntentListener->unregisterListener(this);
    Q_EMIT targetDetectionStopped();
}

void QNearFieldManagerPrivateImpl::newIntent(QJniObject intent)
{
    // This function is called from different thread and is used to move intent to main thread.
    QMetaObject::invokeMethod(this, [this, intent] {
        this->onTargetDiscovered(intent);
    }, Qt::QueuedConnection);
}

QByteArray QNearFieldManagerPrivateImpl::getUid(const QJniObject &intent)
{
    if (!intent.isValid())
        return QByteArray();

    QJniObject tag = QtNfc::getTag(intent);
    if (!tag.isValid())
        return QByteArray();

    return tag.callMethod<jbyte[]>("getId").toContainer();
}

void QNearFieldManagerPrivateImpl::onTargetDiscovered(QJniObject intent)
{
    // Getting UID
    QByteArray uid = getUid(intent);

    // Accepting all targets but only sending signal of requested types.
    QNearFieldTargetPrivateImpl *&target = detectedTargets[uid];
    if (target) {
        target->setIntent(intent);  // Updating existing target
    } else {
        target = new QNearFieldTargetPrivateImpl(intent, uid);

        if (target->accessMethods() & requestedMethod) {
            connect(target, &QNearFieldTargetPrivateImpl::targetDestroyed, this, &QNearFieldManagerPrivateImpl::onTargetDestroyed);
            connect(target, &QNearFieldTargetPrivateImpl::targetLost, this, &QNearFieldManagerPrivateImpl::onTargetLost);
            onTargetDetected(target);
        } else {
            delete target;
            detectedTargets.remove(uid);
        }
    }
}

void QNearFieldManagerPrivateImpl::onTargetDestroyed(const QByteArray &uid)
{
    detectedTargets.remove(uid);
}

QT_END_NAMESPACE
