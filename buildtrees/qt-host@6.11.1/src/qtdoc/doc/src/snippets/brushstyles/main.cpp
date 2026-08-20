// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>

#include "stylewidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    StyleWidget widget;
    widget.show();
    return app.exec();
}
