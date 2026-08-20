// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QQuickView>
#include <QtQuick3D/qquick3d.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat fmt = QQuick3D::idealSurfaceFormat();
    if (QCoreApplication::arguments().contains(QLatin1String("--multisample"))) {
        qDebug("Requesting 4x MSAA on the window");
        fmt.setSamples(4);
    }
    QSurfaceFormat::setDefaultFormat(fmt);

    QQuickView view;
    view.setColor(Qt::black);
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.resize(1024, 768);
    view.setSource(QUrl("qrc:/rhitest.qml"));
    view.show();

    return app.exec();
}
