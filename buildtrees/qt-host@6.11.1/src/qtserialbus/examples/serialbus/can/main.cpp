// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"

#include <QApplication>
#include <QLoggingCategory>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules(u"qt.canbus* = true"_s);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
