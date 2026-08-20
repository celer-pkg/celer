// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include <QLabel>

#include "splitter.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Splitter splitter(Qt::Horizontal);
    splitter.addWidget(new QLabel("Hello"));
    splitter.addWidget(new QLabel("World"));
    Splitter verticalSplitter(Qt::Vertical, &splitter);
    verticalSplitter.addWidget(new QLabel("A"));
    verticalSplitter.addWidget(new QLabel("B"));
    splitter.show();
    return app.exec();
}
