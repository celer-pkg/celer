// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "imagescaling.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    app.setOrganizationName("QtProject");
    app.setApplicationName(QObject::tr("Image Scaling (QtTaskTree)"));

    Images imageView;
    imageView.setWindowTitle(QObject::tr("Image Scaling (QtTaskTree)"));
    imageView.show();

    return app.exec();
}
