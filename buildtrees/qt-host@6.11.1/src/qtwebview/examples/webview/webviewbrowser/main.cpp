// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets/qapplication.h>
#include "browserwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    BrowserWindow browserWindow;
    browserWindow.show();

    return app.exec();
}
