// Copyright (C) 2017 Ford Motor Company.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef PASSTHRUCAN_PASSTHRUCANIO_H
#define PASSTHRUCAN_PASSTHRUCANIO_H

#include "j2534passthru.h"

#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusframe.h>
#include <QByteArray>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariant>

QT_BEGIN_NAMESPACE

class PassThruCanIO : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PassThruCanIO)
public:
    static const uint pollTimeout = 100; // ms

    explicit PassThruCanIO(QObject *parent = nullptr);
    virtual ~PassThruCanIO();

    Q_INVOKABLE void open(const QString &library, const QByteArray &subDev, uint bitRate);
    Q_INVOKABLE void close();
    Q_INVOKABLE void applyConfig(QCanBusDevice::ConfigurationKey key, const QVariant &value);
    Q_INVOKABLE void listen();

    // Internally locked; safe to call directly from any thread.
    bool enqueueMessage(const QCanBusFrame &frame);

Q_SIGNALS:
    void errorOccurred(const QString &description, QCanBusDevice::CanBusError error);
    void messagesReceived(QList<QCanBusFrame> frames);
    void messagesSent(qint64 count);
    void openFinished(bool success);
    void closeFinished();

private:
    bool setMessageFilters(const QList<QCanBusDevice::Filter> &filters);
    bool setConfigValue(J2534::Config::Parameter param, ulong value);
    void pollForMessages();
    bool writeMessages();
    void readMessages(bool writePending);

    J2534::PassThru *       m_passThru      = nullptr;
    J2534::PassThru::Handle m_deviceId      = 0;
    J2534::PassThru::Handle m_channelId     = 0;
    QTimer *                m_idleNotifier  = nullptr;
    QList<J2534::Message> m_ioBuffer;
    QMutex                  m_writeGuard;
    QList<QCanBusFrame>     m_writeQueue;
};

QT_END_NAMESPACE

#endif // PASSTHRUCAN_PASSTHRUCANIO_H
