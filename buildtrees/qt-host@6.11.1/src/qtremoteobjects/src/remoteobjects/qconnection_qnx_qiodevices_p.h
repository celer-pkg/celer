// Copyright (C) 2017-2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:declarations-only

#ifndef QQNXNATIVEIO_P_H
#define QQNXNATIVEIO_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qconnection_qnx_qiodevices.h"
#include "qconnection_qnx_global_p.h"

#include <QtCore/qreadwritelock.h>
#include <QtCore/qscopedpointer.h>

#include "private/qiodevice_p.h"
#include "private/qringbuffer_p.h"

QT_BEGIN_NAMESPACE

class QQnxNativeIoPrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QQnxNativeIo)

    mutable QReadWriteLock ibLock;
    mutable QReadWriteLock obLock;

public:
    QQnxNativeIoPrivate();
    ~QQnxNativeIoPrivate();
    void thread_func();
    bool establishConnection();
    void teardownConnection();
    void stopThread();
    QString serverName;
    int serverId, channelId, connectionId;
    sigevent tx_pulse;
    QAbstractSocket::SocketState state;
    QScopedPointer<QRingBuffer> obuffer;
    MsgType msgType;
    iov_t tx_iov[3], rx_iov[2];
    Thread<QQnxNativeIoPrivate> thread;
};

class QIOQnxSourcePrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QIOQnxSource)

    friend class QQnxNativeServerPrivate;

    mutable QReadWriteLock ibLock;
    mutable QReadWriteLock obLock;

public:
    QIOQnxSourcePrivate(int _rcvid);
    int rcvid;
    QAbstractSocket::SocketState state;
    MsgType msgType;
    sigevent m_event;
    QRingBuffer obuffer;
    QAtomicInt m_serverClosing;
};

QT_END_NAMESPACE

#endif // QQNXNATIVEIO_P_H

