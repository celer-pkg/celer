// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "sudoku.h"

#include <QtWidgets/qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    Sudoku machine;
    MainWindow mainWindow(&machine);

    machine.start();
    mainWindow.show();
    return app.exec();
}
