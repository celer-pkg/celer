// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifdef HAS_MODULE_QT_WIDGETS
# include <QApplication>
#endif
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QtQuick3D/qquick3d.h>

int main(int argc, char *argv[])
{
#ifdef HAS_MODULE_QT_WIDGETS
    QApplication app(argc, argv);
#else
    QGuiApplication app(argc, argv);
#endif
    app.setOrganizationName("The Qt Company");
    app.setOrganizationDomain("qt.io");
    app.setApplicationName("Runtime Asset Loading Example");

    const auto importUrl = argc > 1 ? QUrl::fromLocalFile(argv[1]) : QUrl{};
    if (importUrl.isValid())
        qDebug() << "Importing" << importUrl;

    QSurfaceFormat::setDefaultFormat(QQuick3D::idealSurfaceFormat(4));

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        qWarning() << "Could not find root object in" << url;
        return -1;
    }

    QObject *topLevel = engine.rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);

    if (window)
        window->setProperty("importUrl", importUrl);

    return app.exec();
}
