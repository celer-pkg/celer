// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "imagedownloader.h"
#include "app_environment.h"
#include "import_qml_plugins.h"

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    set_qt_environment();
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/qt/qml/Main/main.qml"_s);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
                [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty())
        return -1;

    ImageDownloader downloader;

    QObject::connect(&downloader, &ImageDownloader::downloadProgress, &app, [&] (double progress) {
        QMetaObject::invokeMethod(engine.rootObjects().at(0), "downloadProgress",
                                  Q_ARG(QVariant, progress));
    });

    QObject::connect(&downloader, &ImageDownloader::downloadStart, &app, [&] (int num) {
        QMetaObject::invokeMethod(engine.rootObjects().at(0), "downloadStart",
                                  Q_ARG(QVariant, num),
                                  Q_ARG(QVariant, downloader.downloadCount()));
    });

    QObject::connect(&downloader, &ImageDownloader::finished, &app, [&] {
        QMetaObject::invokeMethod(engine.rootObjects().at(0), "downloadComplete");
    });

    downloader.downloadImages();

    return app.exec();
}
