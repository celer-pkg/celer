// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include <QtQml/qqmlengine.h>

#include "piewidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    PieWidget graph;
    graph.containerWidget()->show();

    return app.exec();
}
