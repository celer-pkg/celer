// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "clientconsole.h"
#include "ui_clientconsole.h"

#include "sensors.qpb.h"

#include <QMessageBox>

ClientConsole::ClientConsole(QWidget *parent) : QWidget(parent), ui(new Ui::ClientConsole)
{
    ui->setupUi(this);
}

void ClientConsole::onCoordinatesUpdated(const qt::examples::sensors::Coordinates &coord)
{
//! [0]
    ui->latitudeValue->setText(QString::number(coord.latitude(), 'f', 7));
    ui->longitudeValue->setText(QString::number(coord.longitude(), 'f', 7));
    ui->altitudeValue->setText(QString::number(coord.altitude(), 'f', 7));
//! [0]
}

void ClientConsole::onTemperatureUpdated(const qt::examples::sensors::Temperature &temp)
{
    ui->temperature->setText(
            QString("%1 %2")
                    .arg(QString::number(temp.value()))
                    .arg(temp.units() == qt::examples::sensors::Temperature::Unit::Celsius ? 'C'
                                                                                           : 'F'));
}

void ClientConsole::onWarning(const qt::examples::sensors::WarningNotification &warn)
{
    if (!warn.text().isEmpty())
        QMessageBox::information(this, QObject::tr("Important notification"), warn.text());
}

ClientConsole::~ClientConsole()
{
    delete ui;
}

#include "moc_clientconsole.cpp"
