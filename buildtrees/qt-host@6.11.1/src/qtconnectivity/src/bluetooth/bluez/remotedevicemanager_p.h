// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef REMOTEDEVICEMANAGER_P_H
#define REMOTEDEVICEMANAGER_P_H

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

#include <deque>

#include <QList>
#include <QMutex>
#include <QObject>

#include <QtBluetooth/qbluetoothaddress.h>
#include <QtCore/private/qglobal_p.h>


QT_BEGIN_NAMESPACE

// This API is kept a bit more generic in anticipation of further changes in the future.

class RemoteDeviceManager : public QObject
{
    Q_OBJECT
public:
    enum class JobType
    {
        JobDisconnectDevice,
    };

    explicit RemoteDeviceManager(const QBluetoothAddress& localAddress, QObject *parent = nullptr);

    bool isJobInProgress() const { return jobInProgress; }
    bool scheduleJob(JobType job, const QList<QBluetoothAddress> &remoteDevices);

signals:
    void finished();

private slots:
    void runQueue();
    void prepareNextJob();

private:
    void disconnectDevice(const QBluetoothAddress& remote);

    bool jobInProgress = false;
    QBluetoothAddress localAddress;
    std::deque<std::pair<JobType, QBluetoothAddress>> jobQueue;
    QString adapterPath;
};

QT_END_NAMESPACE

#endif // REMOTEDEVICEMANAGER_P_H
