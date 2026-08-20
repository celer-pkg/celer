// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui>
#include "object.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Object object;
    QTimer timer;
    timer.setSingleShot(true);
    timer.connect(&timer, SIGNAL(timeout()), &object, SLOT(print()));
    timer.start(0);
    return app.exec();
}
