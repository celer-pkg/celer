// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SERVERACCEPTANCETHREAD_H
#define SERVERACCEPTANCETHREAD_H

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

#include <QtCore/QMutex>
#include <QtCore/QJniObject>
#include <QtBluetooth/QBluetoothServer>
#include <QtBluetooth/QBluetoothUuid>
#include "qbluetooth.h"
#include "private/qglobal_p.h"

QT_BEGIN_NAMESPACE

class ServerAcceptanceThread : public QObject
{
    Q_OBJECT
public:
    explicit ServerAcceptanceThread(QObject *parent = nullptr);
    ~ServerAcceptanceThread();
    void setServiceDetails(const QBluetoothUuid &uuid, const QString &serviceName,
                           QBluetooth::SecurityFlags securityFlags);


    bool hasPendingConnections() const;
    QJniObject nextPendingConnection();
    void setMaxPendingConnections(int maximumCount);

    void javaThreadErrorOccurred(int errorCode);
    void javaNewSocket(jobject socket);

    void run();
    void stop();
    bool isRunning() const;

signals:
    void newConnection();
    void errorOccurred(QBluetoothServer::Error);

private:
    bool validSetup() const;
    void shutdownPendingConnections();

    QList<QJniObject> pendingSockets;
    mutable QMutex m_mutex;
    QString m_serviceName;
    QBluetoothUuid m_uuid;
    int maxPendingConnections;
    QBluetooth::SecurityFlags secFlags;

    QJniObject javaThread;

};

QT_END_NAMESPACE

#endif // SERVERACCEPTANCETHREAD_H
