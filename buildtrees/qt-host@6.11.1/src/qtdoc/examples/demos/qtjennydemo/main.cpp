// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <nativedispatch.h>
#include "volumeandbrightnessobserver.h"

int main(int argc, char *argv[])
{
    // In some cases Android app might not be able to safely clean all threads
    // while calling exit() and it might crash.
    // This flag avoids calling exit() and lets the Android system handle this,
    // at the cost of not attempting to run global destructors.
    qputenv("QT_ANDROID_NO_EXIT_CALL", "true");

    QGuiApplication app(argc, argv);

    registerNativeInvocationHandler();

    QQmlApplicationEngine engine;
    QObject::connect(
            &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
            []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("qtjenny_consumer", "Main");

    return app.exec();
}
