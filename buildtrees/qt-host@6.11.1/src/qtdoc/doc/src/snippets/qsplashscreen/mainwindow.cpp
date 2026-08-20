// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    QLabel *label = new QLabel(tr("This is the main window."));
    label->setAlignment(Qt::AlignCenter);
    setCentralWidget(label);
}
