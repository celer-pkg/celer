// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui/QGuiApplication>
#include <QtCore/QDir>
#include <QtQuick/QQuickView>
#include <QtQml/QQmlEngine>

int main(int argc, char *argv[])
{
    qputenv("QSG_RHI_BACKEND", "opengl");
    QGuiApplication app(argc, argv);

    QQuickView viewer;

    // The following are needed to make examples run without having to install the module
    // in desktop environments.
#ifdef Q_OS_WIN
    QString extraImportPath(QStringLiteral("%1/../../../../%2"));
#else
    QString extraImportPath(QStringLiteral("%1/../../../%2"));
#endif
    viewer.engine()->addImportPath(extraImportPath.arg(QGuiApplication::applicationDirPath(),
                                      QString::fromLatin1("qml")));
    //! [4]
    QObject::connect(viewer.engine(), &QQmlEngine::quit, &viewer, &QWindow::close);
    //! [4]

    viewer.setTitle(QStringLiteral("Simple Scatter Graph"));

    //! [3]
    viewer.setSource(QUrl("qrc:/qml/qmlscatter/main.qml"));
    //! [3]

    viewer.setResizeMode(QQuickView::SizeRootObjectToView);

    //! [2]
    viewer.showMaximized();
    //! [2]

    return app.exec();
}
