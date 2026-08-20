// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2013 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "service.h"
#include "ui_service.h"

#include <QtBluetooth/qbluetoothaddress.h>
#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/qbluetoothservicediscoveryagent.h>
#include <QtBluetooth/qbluetoothserviceinfo.h>
#include <QtBluetooth/qbluetoothuuid.h>


ServiceDiscoveryDialog::ServiceDiscoveryDialog(const QString &name,
                                               const QBluetoothAddress &address, QWidget *parent)
    :   QDialog(parent), ui(new Ui::ServiceDiscovery)
{
    ui->setupUi(this);

    //Using default Bluetooth adapter
    QBluetoothLocalDevice localDevice;
    QBluetoothAddress adapterAddress = localDevice.address();

    // In case of multiple Bluetooth adapters it is possible to
    // set which adapter will be used by providing MAC Address.
    // Example code:
    //
    // QBluetoothAddress adapterAddress("XX:XX:XX:XX:XX:XX");
    // discoveryAgent = new QBluetoothServiceDiscoveryAgent(adapterAddress, this);

    discoveryAgent = new QBluetoothServiceDiscoveryAgent(adapterAddress, this);
    discoveryAgent->setRemoteAddress(address);

    setWindowTitle(name);

    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered,
            this, &ServiceDiscoveryDialog::addService);
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished,
            ui->status, &QWidget::hide);

    discoveryAgent->start();
}

ServiceDiscoveryDialog::~ServiceDiscoveryDialog()
{
    delete ui;
}

void ServiceDiscoveryDialog::addService(const QBluetoothServiceInfo &info)
{
    if (info.serviceName().isEmpty())
        return;

    QString line = info.serviceName();
    if (!info.serviceDescription().isEmpty())
        line.append("\n\t" + info.serviceDescription());
    if (!info.serviceProvider().isEmpty())
        line.append("\n\t" + info.serviceProvider());

    ui->list->addItem(line);
}
