// Copyright (C) 2016 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtCore/QLoggingCategory>
#include <QtCore/QJniEnvironment>

#include "android/inputstreamthread_p.h"
#include "android/jni_android_p.h"
#include "qbluetoothsocket_android_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

InputStreamThread::InputStreamThread(QBluetoothSocketPrivateAndroid *socket)
    : QObject(), m_socket_p(socket), expectClosure(false)
{
}

bool InputStreamThread::run()
{
    QMutexLocker lock(&m_mutex);

    javaInputStreamThread = QJniObject::construct<QtJniTypes::QtBtInputStreamThread>();
    if (!javaInputStreamThread.isValid() || !m_socket_p->inputStream.isValid())
        return false;

    javaInputStreamThread.callMethod<void>("setInputStream",
                                        m_socket_p->inputStream.object<QtJniTypes::InputStream>());
    javaInputStreamThread.setField("qtObject", reinterpret_cast<jlong>(this));
    javaInputStreamThread.setField<jboolean>("logEnabled", QT_BT_ANDROID().isDebugEnabled());

    javaInputStreamThread.callMethod<void>("start");

    return true;
}

qint64 InputStreamThread::bytesAvailable() const
{
    QMutexLocker locker(&m_mutex);
    return m_socket_p->rxBuffer.size();
}

bool InputStreamThread::canReadLine() const
{
    QMutexLocker locker(&m_mutex);
    return m_socket_p->rxBuffer.canReadLine();
}

qint64 InputStreamThread::readData(char *data, qint64 maxSize)
{
    QMutexLocker locker(&m_mutex);

    if (!m_socket_p->rxBuffer.isEmpty())
        return m_socket_p->rxBuffer.read(data, maxSize);

    return 0;
}

//inside the java thread
void InputStreamThread::javaThreadErrorOccurred(int errorCode)
{
    QMutexLocker lock(&m_mutex);

    if (!expectClosure)
        emit errorOccurred(errorCode);
    else
        emit errorOccurred(-1); // magic error, -1 means error was expected due to expected close()
}

//inside the java thread
void InputStreamThread::javaReadyRead(jbyteArray buffer, int bufferLength)
{
    QJniEnvironment env;

    QMutexLocker lock(&m_mutex);
    char *writePtr = m_socket_p->rxBuffer.reserve(bufferLength);
    env->GetByteArrayRegion(buffer, 0, bufferLength, reinterpret_cast<jbyte*>(writePtr));
    emit dataAvailable();
}

void InputStreamThread::prepareForClosure()
{
    QMutexLocker lock(&m_mutex);
    expectClosure = true;
}

QT_END_NAMESPACE
