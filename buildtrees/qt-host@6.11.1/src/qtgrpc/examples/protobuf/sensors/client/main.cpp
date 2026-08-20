// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>

#include "clientconsole.h"
#include "sensorclient.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    SensorClient client;
    ClientConsole console;
    QObject::connect(&client, &SensorClient::coordinatesUpdated, &console,
                     &ClientConsole::onCoordinatesUpdated);
    QObject::connect(&client, &SensorClient::temperatureUpdated, &console,
                     &ClientConsole::onTemperatureUpdated);
    QObject::connect(&client, &SensorClient::warning, &console, &ClientConsole::onWarning);
    console.show();
    return app.exec();
}
