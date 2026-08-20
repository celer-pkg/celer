// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>

#include "buttonwidget.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QStringList texts;
    texts << "January" << "February" << "March" << "April"
          << "May" << "June" << "July" << "August"
          << "September" << "October" << "November"
          << "December";
    MainWindow *mw = new MainWindow;
    ButtonWidget *buttons = new ButtonWidget(texts, mw);
    mw->setCentralWidget(buttons);
    mw->show();
    QObject::connect(buttons, SIGNAL(clicked(QString)),
                     mw, SLOT(buttonPressed(QString)));
    return app.exec();
}
