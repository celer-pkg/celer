// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtCore/QLoggingCategory>
#include <QtCore/QJniEnvironment>

#include "android/serveracceptancethread_p.h"
#include "android/jni_android_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

ServerAcceptanceThread::ServerAcceptanceThread(QObject *parent) :
    QObject(parent), maxPendingConnections(1)
{
    qRegisterMetaType<QBluetoothServer::Error>();
}

ServerAcceptanceThread::~ServerAcceptanceThread()
{
    Q_ASSERT(!isRunning());
    QMutexLocker lock(&m_mutex);
    shutdownPendingConnections();
}

void ServerAcceptanceThread::setServiceDetails(const QBluetoothUuid &uuid,
                                               const QString &serviceName,
                                               QBluetooth::SecurityFlags securityFlags)
{
    QMutexLocker lock(&m_mutex);
    m_uuid = uuid;
    m_serviceName = serviceName;
    secFlags = securityFlags;
}

bool ServerAcceptanceThread::hasPendingConnections() const
{
    QMutexLocker lock(&m_mutex);
    return !pendingSockets.isEmpty();
}

/*
 * Returns the next pending connection or an invalid JNI object.
 * Note that even a stopped thread may still have pending
 * connections. Pending connections are only terminated upon
 * thread restart or destruction.
 */
QJniObject ServerAcceptanceThread::nextPendingConnection()
{
    QMutexLocker lock(&m_mutex);
    if (pendingSockets.isEmpty())
        return QJniObject();
    else
        return pendingSockets.takeFirst();
}

void ServerAcceptanceThread::setMaxPendingConnections(int maximumCount)
{
    QMutexLocker lock(&m_mutex);
    maxPendingConnections = maximumCount;
}

void ServerAcceptanceThread::run()
{
    QMutexLocker lock(&m_mutex);

    if (!validSetup()) {
        qCWarning(QT_BT_ANDROID) << "Invalid Server Socket setup";
        return;
    }

    if (isRunning()) {
        stop();
        shutdownPendingConnections();
    }

    javaThread = QJniObject::construct<QtJniTypes::QtBtSocketServer>(
                     QNativeInterface::QAndroidApplication::context());
    if (!javaThread.isValid())
        return;

    javaThread.setField("qtObject", reinterpret_cast<jlong>(this));
    javaThread.setField<jboolean>("logEnabled", QT_BT_ANDROID().isDebugEnabled());

    QString tempUuid = m_uuid.toString(QUuid::WithoutBraces);

    QJniObject uuidString = QJniObject::fromString(tempUuid);
    QJniObject serviceNameString = QJniObject::fromString(m_serviceName);
    bool isSecure = !(secFlags == QBluetooth::SecurityFlags(QBluetooth::Security::NoSecurity));
    javaThread.callMethod<void>("setServiceDetails",
                                uuidString.object<jstring>(),
                                serviceNameString.object<jstring>(),
                                isSecure);
    javaThread.callMethod<void>("start");
}

void ServerAcceptanceThread::stop()
{
    if (javaThread.isValid()) {
        qCDebug(QT_BT_ANDROID) << "Closing server socket";
        javaThread.callMethod<void>("close");
    }
}

bool ServerAcceptanceThread::isRunning() const
{
    if (javaThread.isValid())
        return javaThread.callMethod<jboolean>("isAlive");

    return false;
}

//Runs inside the java thread
void ServerAcceptanceThread::javaThreadErrorOccurred(int errorCode)
{
    qCDebug(QT_BT_ANDROID) << "JavaThread error:" << errorCode;
    emit errorOccurred(QBluetoothServer::InputOutputError);
}

//Runs inside the Java thread
void ServerAcceptanceThread::javaNewSocket(jobject s)
{
    QMutexLocker lock(&m_mutex);

    QJniObject socket(s);
    if (!socket.isValid())
       return;

    if (pendingSockets.size() < maxPendingConnections) {
        qCDebug(QT_BT_ANDROID) << "New incoming java socket detected";
        pendingSockets.append(socket);
        emit newConnection();
    } else {
        qCWarning(QT_BT_ANDROID) << "Refusing connection due to limited pending socket queue";
        socket.callMethod<void>("close");
    }
}

bool ServerAcceptanceThread::validSetup() const
{
    return (!m_uuid.isNull() && !m_serviceName.isEmpty());
}

void ServerAcceptanceThread::shutdownPendingConnections()
{
    while (!pendingSockets.isEmpty()) {
        QJniObject socket = pendingSockets.takeFirst();
        socket.callMethod<void>("close");
    }
}

QT_END_NAMESPACE
