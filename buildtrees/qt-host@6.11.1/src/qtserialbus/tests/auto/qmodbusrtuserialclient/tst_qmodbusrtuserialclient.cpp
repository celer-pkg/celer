// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtSerialBus/qmodbusrtuserialclient.h>

#include <QtTest/QtTest>

class tst_QModbusRtuSerialClient : public QObject
{
    Q_OBJECT

private slots:
    void testInterFrameDelay()
    {
        QModbusRtuSerialClient qmrsm;
        QCOMPARE(qmrsm.interFrameDelay(), 2000);
        qmrsm.setInterFrameDelay(1000);
        QCOMPARE(qmrsm.interFrameDelay(), 2000);
        qmrsm.setInterFrameDelay(3000);
        QCOMPARE(qmrsm.interFrameDelay(), 3000);
        qmrsm.setInterFrameDelay(-1);
        QCOMPARE(qmrsm.interFrameDelay(), 2000);
    }
};

QTEST_MAIN(tst_QModbusRtuSerialClient)

#include "tst_qmodbusrtuserialclient.moc"
