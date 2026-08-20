// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DEVICE_H
#define DEVICE_H

#include <QtBluetooth/qbluetoothlocaldevice.h>

#include <QtWidgets/qdialog.h>

QT_BEGIN_NAMESPACE
class QBluetoothAddress;
class QBluetoothDeviceDiscoveryAgent;
class QBluetoothDeviceInfo;
class QListWidgetItem;

namespace Ui {
    class DeviceDiscovery;
}
QT_END_NAMESPACE

class DeviceDiscoveryDialog : public QDialog
{
    Q_OBJECT

public:
    DeviceDiscoveryDialog(QWidget *parent = nullptr);
    ~DeviceDiscoveryDialog();

public slots:
    void addDevice(const QBluetoothDeviceInfo &info);
    void on_power_clicked(bool clicked);
    void on_discoverable_clicked(bool clicked);
    void displayPairingMenu(const QPoint &pos);
    void pairingDone(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing);
private slots:
    void startScan();
    void stopScan();
    void scanFinished();
    void itemActivated(QListWidgetItem *item);
    void hostModeStateChanged(QBluetoothLocalDevice::HostMode mode);

private:
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    QBluetoothLocalDevice *localDevice;
    Ui::DeviceDiscovery *ui;
};

#endif
