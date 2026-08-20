// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

/*
    main.cpp

    An example of a main window application that used a subclassed model
    and view to display data from sound files.
*/

#include <QApplication>

#include "model.h"
#include "view.h"
#include "window.h"

/*!
    The main function for the linear model example. This creates and
    populates a model with long integers then displays the contents of the
    model using a QListView widget.
*/

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow *window = new MainWindow;

    window->show();
    return app.exec();
}
