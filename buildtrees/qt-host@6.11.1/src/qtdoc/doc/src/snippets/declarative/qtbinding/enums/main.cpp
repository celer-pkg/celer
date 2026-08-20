// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include <QtCore>
#include <QtDeclarative>

#include "imageviewer.h"

ImageViewer::ImageViewer(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
{
    QTimer::singleShot(0, this, SLOT(emitSignals()));
}

ImageViewer::Status ImageViewer::status() const
{
    return Ready;
}

void ImageViewer::emitSignals()
{
    emit statusChanged();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qmlRegisterType<ImageViewer>("MyLibrary", 1, 0, "ImageViewer");

    QDeclarativeView view;
    view.setSource(QUrl::fromLocalFile("standalone.qml"));
    view.show();

    return app.exec();
}

