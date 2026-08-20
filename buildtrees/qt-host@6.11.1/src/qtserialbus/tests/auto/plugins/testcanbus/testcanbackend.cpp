// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testcanbackend.h"

#include <QtCore/qdatetime.h>
#include <QtCore/qdebug.h>
#include <QtCore/qtimer.h>

QT_BEGIN_NAMESPACE

TestCanBackend::TestCanBackend() :
    simulateReceivingTimer(new QTimer(this))
{
    connect(simulateReceivingTimer, &QTimer::timeout, this, [this]() {
        const quint64 timeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
        QCanBusFrame dummyFrame(12, "def");
        dummyFrame.setTimeStamp(QCanBusFrame::TimeStamp::fromMicroSeconds(timeStamp * 1000));

        enqueueReceivedFrames({dummyFrame});
    });
}

bool TestCanBackend::open()
{
    simulateReceivingTimer->start(1000);
    setState(QCanBusDevice::ConnectedState);
    return true;
}

void TestCanBackend::close()
{
    simulateReceivingTimer->stop();
    setState(QCanBusDevice::UnconnectedState);
}

bool TestCanBackend::writeFrame(const QCanBusFrame &data)
{
    qDebug("DummyBackend::writeFrame: %ls", qUtf16Printable(data.toString()));
    return true;
}

QString TestCanBackend::interpretErrorFrame(const QCanBusFrame &/*errorFrame*/)
{
    return QString();
}

QList<QCanBusDeviceInfo> TestCanBackend::interfaces()
{
    return {createDeviceInfo(QStringLiteral("testcan"), QStringLiteral("can0"), true, true)};
}

QT_END_NAMESPACE
