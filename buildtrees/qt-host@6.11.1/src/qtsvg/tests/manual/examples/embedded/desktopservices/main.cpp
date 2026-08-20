// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include "desktopwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    DesktopWidget* myWidget = new DesktopWidget(0);
    myWidget->showMaximized();

    return app.exec();
}

// End of file
