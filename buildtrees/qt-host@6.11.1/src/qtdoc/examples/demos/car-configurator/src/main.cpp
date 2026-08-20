// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "app_environment.h"
#include "import_qml_plugins.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    using namespace Qt::StringLiterals;
    set_qt_environment();

#if defined(Q_OS_ANDROID)
    QSurfaceFormat format;
    format.setSamples(4);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 1);
    QSurfaceFormat::setDefaultFormat(format);
    QGuiApplication::setAttribute(Qt::AA_UseOpenGLES);
#endif

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/qt/qml/content/App.qml"_s);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
                [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.addImportPath(":/qt/qml");
    engine.addImportPath(":/qt/qml/asset_imports");
    engine.addImportPath(":/qt/qml/content");
    engine.addImportPath(":/qt/qml/imports");

    engine.load(url);

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
