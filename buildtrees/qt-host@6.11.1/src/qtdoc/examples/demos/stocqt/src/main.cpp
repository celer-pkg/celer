// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QtGraphs/qutils.h>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickView>

int main(int argc, char *argv[])
{
    qputenv("QT_QUICK_CONTROLS_CONF", ":/qtquickcontrols2.conf");
    QGuiApplication app(argc, argv);

    QQuickView viewer;
    viewer.setFormat(qDefaultSurfaceFormat(true));

#ifdef Q_OS_WIN
    QString extraImportPath(QStringLiteral("%1/../../../../%2"));
#else
    QString extraImportPath(QStringLiteral("%1/../../../%2"));
#endif
    viewer.engine()->addImportPath(
        extraImportPath.arg(QGuiApplication::applicationDirPath(), QString::fromLatin1("qml")));
    QObject::connect(viewer.engine(), &QQmlEngine::quit, &viewer, &QWindow::close);
    viewer.setTitle(QStringLiteral("StocQt"));
    viewer.setSource(QUrl("qrc:/Main.qml"));
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);
    viewer.show();

    return app.exec();
}
