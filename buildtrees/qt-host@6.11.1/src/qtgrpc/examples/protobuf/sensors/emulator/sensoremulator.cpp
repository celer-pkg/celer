// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "sensoremulator.h"

#include <QDebug>
#include <QNetworkDatagram>

#include "tlv.qpb.h"
#include "sensors.qpb.h"

using namespace qt::examples::sensors;

namespace {
QByteArray makeTlvMessage(QProtobufSerializer *serializer, const QByteArray &data,
                          tlv::MessageTypeGadget::MessageType type)
{
//! [0]
    Q_ASSERT(serializer != nullptr);
    tlv::TlvMessage msg;
    msg.setType(type);
    msg.setValue(data);
    return msg.serialize(serializer);
//! [0]
}
} // namespace

SensorEmulator::SensorEmulator(QObject *parent) : QObject(parent) { }

void SensorEmulator::send(const QByteArray &data)
{
    if (m_socket.writeDatagram(data, QHostAddress::LocalHost, 65500) == -1)
        qWarning() << "Unable to send data of size: " << data.size() << "to UDP port 65500";
}

void SensorEmulator::sendCoordinates(const Coordinates &coords)
{
    send(makeTlvMessage(&m_serializer, coords.serialize(&m_serializer),
                        tlv::MessageTypeGadget::MessageType::Coordinates));
}

void SensorEmulator::sendTemperature(const Temperature &temp)
{
    send(makeTlvMessage(&m_serializer, temp.serialize(&m_serializer),
                        tlv::MessageTypeGadget::MessageType::Temperature));
}

void SensorEmulator::sendWarning(const WarningNotification &warn)
{
    send(makeTlvMessage(&m_serializer, warn.serialize(&m_serializer),
                        tlv::MessageTypeGadget::MessageType::WarningNotification));
}

#include "moc_sensoremulator.cpp"
