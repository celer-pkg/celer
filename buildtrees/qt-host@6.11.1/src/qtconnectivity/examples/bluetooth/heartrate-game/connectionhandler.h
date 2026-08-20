// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include <QBluetoothLocalDevice>

#include <QObject>

#include <QQmlEngine>

class ConnectionHandler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool alive READ alive NOTIFY deviceChanged)
    Q_PROPERTY(bool hasPermission READ hasPermission NOTIFY deviceChanged)
    Q_PROPERTY(QString name READ name NOTIFY deviceChanged)
    Q_PROPERTY(QString address READ address NOTIFY deviceChanged)
    Q_PROPERTY(bool requiresAddressType READ requiresAddressType CONSTANT)

    QML_ELEMENT
public:
    explicit ConnectionHandler(QObject *parent = nullptr);

    bool alive() const;
    bool hasPermission() const;
    bool requiresAddressType() const;
    QString name() const;
    QString address() const;

signals:
    void deviceChanged();

private slots:
    void hostModeChanged(QBluetoothLocalDevice::HostMode mode);
    void initLocalDevice();

private:
    QBluetoothLocalDevice *m_localDevice = nullptr;
    bool m_hasPermission = false;
};

#endif // CONNECTIONHANDLER_H
