// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CONNECTPAGE_H
#define CONNECTPAGE_H

#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtWidgets/QWidget>
#include <QtWidgets/QListWidget>
#include <QtCore/QStringListModel>
#include <QtCore/QPointer>

class BLEIoDevice;

QT_BEGIN_NAMESPACE
class QBluetoothDeviceDiscoveryAgent;
class QLowEnergyController;
class QLowEnergyService;

namespace Ui {
class ConnectPage;
}
QT_END_NAMESPACE

class ConnectPage : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectPage(QWidget *parent = nullptr);
    ~ConnectPage() override;
    void startScanning();
    void disconnectFromDevice();

signals:
    void showMessage(const QString &message);
    void connected(BLEIoDevice *ioDevice);
    void disconnected();

private:
    void connectToDevice(const QBluetoothDeviceInfo &device);
    void connectNode();

private slots:
    void refreshDevices();

private:
    Ui::ConnectPage *ui;
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent = nullptr;
    QLowEnergyController *m_controller = nullptr;
    QPointer<QLowEnergyService> m_service;
    QList<QBluetoothDeviceInfo> m_leDevices;
};

#endif // CONNECTPAGE_H
