// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TESTCANBACKEND_H
#define TESTCANBACKEND_H

#include <QtSerialBus/qcanbusdevice.h>

QT_BEGIN_NAMESPACE

class QTimer;

class TestCanBackend : public QCanBusDevice
{
    Q_OBJECT
public:
    explicit TestCanBackend();

    bool open() override;
    void close() override;

    bool writeFrame(const QCanBusFrame &data) override;

    QString interpretErrorFrame(const QCanBusFrame &) override;

    static QList<QCanBusDeviceInfo> interfaces();

private:
    QTimer *simulateReceivingTimer = nullptr;
};

QT_END_NAMESPACE

#endif // TESTCANBACKEND_H
