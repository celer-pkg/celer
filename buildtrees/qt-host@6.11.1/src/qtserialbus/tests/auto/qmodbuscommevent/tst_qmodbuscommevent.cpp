// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <private/qmodbuscommevent_p.h>

#include <QtTest/QtTest>

class tst_QModbusCommEvent : public QObject
{
    Q_OBJECT

private slots:
    void testConstructor()
    {
        QCOMPARE(quint8(QModbusCommEvent(QModbusCommEvent::SentEvent)), quint8(0x40));
        QCOMPARE(quint8(QModbusCommEvent(QModbusCommEvent::ReceiveEvent)), quint8(0x80));
        QCOMPARE(quint8(QModbusCommEvent(QModbusCommEvent::EnteredListenOnlyMode)), quint8(0x04));
        QCOMPARE(quint8(QModbusCommEvent(QModbusCommEvent::InitiatedCommunicationRestart)),
                 quint8(0x00));
    }

    void testAssignmentOperator()
    {
        QModbusCommEvent e = QModbusCommEvent::SentEvent;
        QCOMPARE(quint8(e), quint8(0x40));
        e = QModbusCommEvent::ReceiveEvent;
        QCOMPARE(quint8(e), quint8(0x80));
        e = QModbusCommEvent::EnteredListenOnlyMode;
        QCOMPARE(quint8(e), quint8(0x04));
        e = QModbusCommEvent::InitiatedCommunicationRestart;
        QCOMPARE(quint8(e), quint8(0x00));
    }

    void testOperatorOr()
    {
        QModbusCommEvent sent = QModbusCommEvent::SentEvent;
        QCOMPARE(quint8(sent | QModbusCommEvent::SendFlag::CurrentlyInListenOnlyMode),
                 quint8(0x60));

        QModbusCommEvent receive = QModbusCommEvent::ReceiveEvent;
        QCOMPARE(quint8(receive | QModbusCommEvent::ReceiveFlag::CurrentlyInListenOnlyMode),
                 quint8(0xa0));
    }

    void testOperatorOrAssign()
    {
        QModbusCommEvent sent = QModbusCommEvent::SentEvent;
        sent |= QModbusCommEvent::SendFlag::ReadExceptionSent;
        QCOMPARE(quint8(sent), quint8(0x41));

        QModbusCommEvent receive = QModbusCommEvent::ReceiveEvent;
        receive |= QModbusCommEvent::ReceiveFlag::CurrentlyInListenOnlyMode;
        QCOMPARE(quint8(receive), quint8(0xa0));
    }
};

QTEST_MAIN(tst_QModbusCommEvent)

#include "tst_qmodbuscommevent.moc"
