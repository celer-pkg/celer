// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QFontDatabase>

int g_customFontId = 0;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Enable this to test with OpenGL backend so QQuickPaintedItem
    // FramebufferObject renderTarget can be enabled (with Qt >= 6.9)
    //QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    //QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSamples(4);
    // Enable this to disable vsync animations
    format.setSwapInterval(0);
    QSurfaceFormat::setDefaultFormat(format);

    g_customFontId = QFontDatabase::addApplicationFont(":/qml/fonts/Roboto-Regular.ttf");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
