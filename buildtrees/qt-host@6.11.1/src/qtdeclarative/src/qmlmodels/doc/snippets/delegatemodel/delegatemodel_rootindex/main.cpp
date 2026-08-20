// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include <QGuiApplication>
#include <QFileSystemModel>
#include <QQmlEngine>
#include <QQuickView>

//![0]
int main(int argc, char ** argv)
{
    QGuiApplication app(argc, argv);

    QQuickView view;

    QFileSystemModel model;
    // start populating the model (doesn't change the model's root)
    model.setRootPath(QDir::currentPath());

    qmlRegisterSingletonInstance("FileSystemModule", 1, 0, "FileSystemModel", &model);
    view.setSource(QUrl::fromLocalFile("view.qml"));
    view.show();

    return app.exec();
}
//![0]
