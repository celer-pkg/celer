// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "rep_heater_replica.h"

#include "heaterview.h"
#include "ui_heaterview.h"
#include "bleiodevice.h"

using namespace Qt::Literals::StringLiterals;

HeaterView::HeaterView(BLEIoDevice *ioDevice, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HeaterView)
{
    ui->setupUi(this);
    connect(ui->closeButton, &QPushButton::clicked, this, &HeaterView::closeMe);

     // Signal the server that we are ready to start
    ioDevice->write(BLEIoDevice::CLIENT_READY_SIGNAL);

    m_node.addClientSideConnection(ioDevice);
    m_heater.reset(m_node.acquire<HeaterReplica>());

    const auto updateHeaterPowerUi = [this](){
        ui->toggleHeater->setText(m_heater->heaterPoweredOn()
                                   ? "ON (switch OFF)"_L1
                                   : "OFF (switch ON)"_L1);
    };

    const auto updateTemperatureUi = [this](){
        ui->temperatureDigits->display(m_heater->currentTemperature());
    };

    // Initial updates and connect UI updates to changes in values
    updateHeaterPowerUi();
    updateTemperatureUi();
    QObject::connect(m_heater.get(), &HeaterReplica::heaterPoweredOnChanged,
                     this, updateHeaterPowerUi);
    QObject::connect(m_heater.get(), &HeaterReplica::currentTemperatureChanged,
                     this, updateTemperatureUi);

    // When user toggles heater ON/OFF, push the value to server/source
    QObject::connect(ui->toggleHeater, &QPushButton::clicked, this, [this]() {
        m_heater->pushHeaterPoweredOn(!m_heater->heaterPoweredOn());
    });
}

HeaterView::~HeaterView()
{
    delete ui;
}
