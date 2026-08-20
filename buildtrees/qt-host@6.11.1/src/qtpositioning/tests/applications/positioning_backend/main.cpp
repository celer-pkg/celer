// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include "widget.h"
#include "logwidget.h"
#include <QLabel>

#include <QApplication>
#include <QtWidgets>
#include <QLoggingCategory>

int main(int argc, char *argv[])
{
    //QLoggingCategory::setFilterRules("qt.positioning.*=true");
    QApplication a(argc, argv);

    LogWidget *log = new LogWidget;
    Widget *w1 = new Widget(log);
    Widget *w2 = new Widget(log);

    QTabWidget tabWidget;
    tabWidget.setTabPosition(QTabWidget::South);

    tabWidget.addTab(w1, "Instance 1");
    tabWidget.addTab(w2, "Instance 2");
    tabWidget.addTab(log, "Logs");

    tabWidget.show();
    return a.exec();
}
