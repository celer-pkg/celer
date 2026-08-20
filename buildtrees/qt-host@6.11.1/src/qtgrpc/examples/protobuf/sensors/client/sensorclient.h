// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SENSORCLIENT_H
#define SENSORCLIENT_H

#include <QObject>

#include <QProtobufSerializer>
#include <QUdpSocket>

namespace qt::examples::sensors {
class Coordinates;
class Temperature;
class WarningNotification;
} // namespace qt::examples::sensors

class SensorClient : public QObject
{
    Q_OBJECT
public:
    explicit SensorClient(QObject *parent = nullptr);
    void receive();

signals:
    void coordinatesUpdated(const qt::examples::sensors::Coordinates &);
    void temperatureUpdated(const qt::examples::sensors::Temperature &);
    void warning(const qt::examples::sensors::WarningNotification &);

private:
    QUdpSocket m_client;
    QProtobufSerializer m_serializer;
};

#endif // SENSORCLIENT_H
