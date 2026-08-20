// Copyright (C) 2024 Stan Morris
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QQmlApplicationEngine>


int main(int argc, char *argv[])
{
    // Uncomment to show performance metrics
    //qputenv("QSG_RENDERER_DEBUG", "render");  // Show number of batches
    //qputenv("QSG_VISUALIZE", "batches");      // Show GPU batches
    //qputenv("QSG_RENDER_TIMING", "1");        // Show GPU timings
    //qputenv("QSG_VISUALIZE", "clip");         // Show Items with clip == true
    //qputenv("QSG_VISUALIZE", "changes");      // Show updating items
    //qputenv("QSG_VISUALIZE", "overdraw");     // Show off-screen Items, hidden Items
    //qputenv("QSG_RENDER_LOOP", "threaded");   // types: basic, windows, threaded

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);
    engine.loadFromModule("VertexSpiral", "Main");

    return app.exec();
}
