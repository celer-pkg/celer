// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "emulatorconsole.h"
#include "ui_emulatorconsole.h"

#include <QDoubleValidator>
#include <QIntValidator>

#include "sensors.qpb.h"

EmulatorConsole::EmulatorConsole(QWidget *parent) : QWidget(parent), ui(new Ui::EmulatorConsole)
{
    ui->setupUi(this);
    auto validator = new QDoubleValidator(-90, 90, 7, ui->latitudeValue);
    validator->setLocale(QLocale::c());
    ui->latitudeValue->setValidator(validator);

    validator = new QDoubleValidator(-180, 180, 7, ui->longitudeValue);
    validator->setLocale(QLocale::c());
    ui->longitudeValue->setValidator(validator);

    validator = new QDoubleValidator(-1000, 1000, 7, ui->altitudeValue);
    validator->setLocale(QLocale::c());
    ui->altitudeValue->setValidator(validator);
    ui->temperatureValue->setValidator(new QIntValidator(-50, 50, ui->temperatureValue));
// ![0]
    QObject::connect(ui->sendCoordinates, &QPushButton::clicked, this, [this]() {
        qt::examples::sensors::Coordinates coord;
        coord.setLatitude(ui->latitudeValue->text().toDouble());
        coord.setLongitude(ui->longitudeValue->text().toDouble());
        coord.setAltitude(ui->altitudeValue->text().toDouble());
        emit coordinatesUpdated(coord);
    });
// ![0]

    QObject::connect(ui->sendTemperature, &QPushButton::clicked, this, [this]() {
        qt::examples::sensors::Temperature temp;
        temp.setValue(ui->temperatureValue->text().toInt());
        temp.setUnits(ui->celciusRadio->isChecked()
                              ? qt::examples::sensors::Temperature::Unit::Celsius
                              : qt::examples::sensors::Temperature::Unit::Farenheit);
        emit temperatureUpdated(temp);
    });

    QObject::connect(ui->sendMessage, &QPushButton::clicked, this, [this]() {
        qt::examples::sensors::WarningNotification warn;
        warn.setText(ui->warningText->toPlainText());
        emit warning(warn);
    });
}

EmulatorConsole::~EmulatorConsole()
{
    delete ui;
}

#include "moc_emulatorconsole.cpp"
