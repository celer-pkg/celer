// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SENSOREMULATOR_H
#define SENSOREMULATOR_H

#include <QObject>
#include <QUdpSocket>
#include <QProtobufSerializer>

namespace qt::examples::sensors {
class Coordinates;
class Temperature;
class WarningNotification;
} // namespace qt::examples::sensors
class SensorEmulator : public QObject
{
    Q_OBJECT
public:
    explicit SensorEmulator(QObject *parent = nullptr);

    void sendCoordinates(const qt::examples::sensors::Coordinates &coords);
    void sendTemperature(const qt::examples::sensors::Temperature &temp);
    void sendWarning(const qt::examples::sensors::WarningNotification &warn);

private:
    void send(const QByteArray &data);

    QUdpSocket m_socket;
    QProtobufSerializer m_serializer;
};

#endif // SENSOREMULATOR_H
