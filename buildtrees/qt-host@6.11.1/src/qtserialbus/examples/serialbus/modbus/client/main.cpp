// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QLoggingCategory>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(u"Modbus Client Example"_s);
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption verboseOption(u"verbose"_s, u"Verbose mode"_s);
    parser.addOption(verboseOption);
    parser.process(a);

    if (parser.isSet(verboseOption))
        QLoggingCategory::setFilterRules(u"qt.modbus* = true"_s);

    MainWindow w;
    w.show();

    return a.exec();
}
