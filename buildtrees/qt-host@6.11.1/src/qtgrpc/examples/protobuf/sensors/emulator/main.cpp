// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "emulatorconsole.h"

#include <QApplication>
#include "sensoremulator.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    SensorEmulator emul;
    EmulatorConsole console;
    QObject::connect(&console, &EmulatorConsole::coordinatesUpdated, &emul,
                     &SensorEmulator::sendCoordinates);
    QObject::connect(&console, &EmulatorConsole::temperatureUpdated, &emul,
                     &SensorEmulator::sendTemperature);
    QObject::connect(&console, &EmulatorConsole::warning, &emul, &SensorEmulator::sendWarning);
    console.show();
    return app.exec();
}
