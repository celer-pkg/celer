// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui/qguiapplication.h>
#include <QtQml/qqmlengine.h>
#include <QtQuick/qquickview.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickView viewer;

    viewer.setTitle(QStringLiteral("Cockpit"));

    viewer.setMaximumSize({1280, 720});
    viewer.setMinimumSize({1280, 720});
    viewer.setSource(QUrl("qrc:/qml/cockpit/main.qml"));
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);
    viewer.setColor("black");
    viewer.show();

    return app.exec();
}
