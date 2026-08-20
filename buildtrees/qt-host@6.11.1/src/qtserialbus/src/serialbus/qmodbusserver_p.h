// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QMODBUSERVER_P_H
#define QMODBUSERVER_P_H

#include <QtSerialBus/qmodbusdataunit.h>
#include <QtSerialBus/qmodbusserver.h>

#include <private/qmodbuscommevent_p.h>
#include <private/qmodbusdevice_p.h>
#include <private/qmodbus_symbols_p.h>

#include <array>
#include <deque>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QModbusServerPrivate : public QModbusDevicePrivate
{
    Q_DECLARE_PUBLIC(QModbusServer)

public:
    enum Counter {
        CommEvent = 0x0001,
        BusMessage = Diagnostics::ReturnBusMessageCount,
        BusCommunicationError = Diagnostics::ReturnBusCommunicationErrorCount,
        BusExceptionError = Diagnostics::ReturnBusExceptionErrorCount,
        ServerMessage = Diagnostics::ReturnServerMessageCount,
        ServerNoResponse = Diagnostics::ReturnServerNoResponseCount,
        ServerNAK = Diagnostics::ReturnServerNAKCount,
        ServerBusy = Diagnostics::ReturnServerBusyCount,
        BusCharacterOverrun = Diagnostics::ReturnBusCharacterOverrunCount
    };

    QModbusServerPrivate()
        : m_counters()
    {
    }

    bool setMap(const QModbusDataUnitMap &map);

    void resetCommunicationCounters() { m_counters.fill(0u); }
    void incrementCounter(QModbusServerPrivate::Counter counter) { m_counters[counter]++; }

    QModbusResponse processRequest(const QModbusPdu &request);

    QModbusResponse processReadCoilsRequest(const QModbusRequest &request);
    QModbusResponse processReadDiscreteInputsRequest(const QModbusRequest &request);
    QModbusResponse readBits(const QModbusPdu &request, QModbusDataUnit::RegisterType unitType);

    QModbusResponse processReadHoldingRegistersRequest(const QModbusRequest &request);
    QModbusResponse processReadInputRegistersRequest(const QModbusRequest &request);
    QModbusResponse readBytes(const QModbusPdu &request, QModbusDataUnit::RegisterType unitType);

    QModbusResponse processWriteSingleCoilRequest(const QModbusRequest &request);
    QModbusResponse processWriteSingleRegisterRequest(const QModbusRequest &request);
    QModbusResponse writeSingle(const QModbusPdu &request, QModbusDataUnit::RegisterType unitType);

    QModbusResponse processReadExceptionStatusRequest(const QModbusRequest &request);
    QModbusResponse processDiagnosticsRequest(const QModbusRequest &request);
    QModbusResponse processGetCommEventCounterRequest(const QModbusRequest &request);
    QModbusResponse processGetCommEventLogRequest(const QModbusRequest &request);
    QModbusResponse processWriteMultipleCoilsRequest(const QModbusRequest &request);
    QModbusResponse processWriteMultipleRegistersRequest(const QModbusRequest &request);
    QModbusResponse processReportServerIdRequest(const QModbusRequest &request);
    QModbusResponse processMaskWriteRegisterRequest(const QModbusRequest &request);
    QModbusResponse processReadWriteMultipleRegistersRequest(const QModbusRequest &request);
    QModbusResponse processReadFifoQueueRequest(const QModbusRequest &request);
    QModbusResponse processEncapsulatedInterfaceTransportRequest(const QModbusRequest &request);

    void storeModbusCommEvent(const QModbusCommEvent &eventByte);

    int m_serverAddress = 1;
    std::array<quint16, 20> m_counters;
    QHash<int, QVariant> m_serverOptions;
    QModbusDataUnitMap m_modbusDataUnitMap;
    std::deque<quint8> m_commEventLog;
};

QT_END_NAMESPACE

#endif // QMODBUSERVER_P_H
