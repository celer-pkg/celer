// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui>
#include "paintwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    PaintWidget window;
    window.show();
    return app.exec();
}
