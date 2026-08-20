// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QThread>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
            &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
            []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("qtsplashscreeninandroid", "Main");

    // Hide splash screen after 2 seconds, long visibility time used for demonstration purposes.
//! [Hide splash screen call]
    QNativeInterface::QAndroidApplication::hideSplashScreen(2000);
//! [Hide splash screen call]
    return app.exec();
}
