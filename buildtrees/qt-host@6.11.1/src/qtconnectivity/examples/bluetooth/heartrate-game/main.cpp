// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "connectionhandler.h"
#include "devicefinder.h"
#include "devicehandler.h"
#include "heartrate-global.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QLoggingCategory>

#include <QGuiApplication>

#include <QQmlApplicationEngine>

using namespace Qt::StringLiterals;

bool simulator = false;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(u"Bluetooth Low Energy Heart Rate Game"_s);
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption simulatorOption(u"simulator"_s, u"Simulator"_s);
    parser.addOption(simulatorOption);

    QCommandLineOption verboseOption(u"verbose"_s, u"Verbose mode"_s);
    parser.addOption(verboseOption);
    parser.process(app);

    if (parser.isSet(verboseOption))
        QLoggingCategory::setFilterRules(u"qt.bluetooth* = true"_s);
    simulator = parser.isSet(simulatorOption);

    ConnectionHandler connectionHandler;
    DeviceHandler deviceHandler;
    DeviceFinder deviceFinder(&deviceHandler);

    QQmlApplicationEngine engine;
    engine.setInitialProperties({
        {u"connectionHandler"_s, QVariant::fromValue(&connectionHandler)},
        {u"deviceFinder"_s, QVariant::fromValue(&deviceFinder)},
        {u"deviceHandler"_s, QVariant::fromValue(&deviceHandler)}
    });

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     []() { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("HeartRateGame", "Main");

    return app.exec();
}
