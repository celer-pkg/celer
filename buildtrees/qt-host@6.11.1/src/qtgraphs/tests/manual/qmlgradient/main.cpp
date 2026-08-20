// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGui/QGuiApplication>
#include <QtCore/QDir>
#include <QtQuick/QQuickView>
#include <QtQml/QQmlEngine>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    QQuickView viewer;

    const QUrl url(QStringLiteral("qrc:/qml/qmlgradient/main.qml"));

    // The following are needed to make examples run without having to install the module
    // in desktop environments.
#ifdef Q_OS_WIN
    QString extraImportPath(QStringLiteral("%1/../../../%2"));
#else
    QString extraImportPath(QStringLiteral("%1/../../%2"));
#endif

    viewer.engine()->addImportPath(extraImportPath.arg(QGuiApplication::applicationDirPath(),
                                      QString::fromLatin1("qml")));
    QObject::connect( viewer.engine(), &QQmlEngine::quit, &viewer, &QWindow::close);
    viewer.setSource(url);
    viewer.show();
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);
    return app.exec();
}
