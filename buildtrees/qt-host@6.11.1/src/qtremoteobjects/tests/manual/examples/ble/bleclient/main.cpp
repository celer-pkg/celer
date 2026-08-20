// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"

#include <QtRemoteObjects/QRemoteObjectNode>
#include <QtWidgets/QApplication>
#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption verboseOption("verbose", "Verbose mode");
    parser.addOption(verboseOption);
    parser.process(app);
    if (parser.isSet(verboseOption)) {
        QLoggingCategory::setFilterRules("qt.remoteobjects* = true"_L1);
        QLoggingCategory::setFilterRules("qt.bluetooth* = true"_L1);
    }

    MainWindow w;
    w.show();
    return app.exec();
}
