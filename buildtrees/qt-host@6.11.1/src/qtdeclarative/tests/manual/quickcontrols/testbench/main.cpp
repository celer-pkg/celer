// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QDebug>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QSettings>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QtQuickControls2/private/qquickstyle_p.h>

int main(int argc, char *argv[])
{
    QGuiApplication::setApplicationName("testbench");
    QGuiApplication::setOrganizationName("QtProject");
    QGuiApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    QGuiApplication::setAttribute(Qt::AA_DontUseNativeMenuWindows);

    QGuiApplication app(argc, argv);

    QSettings settings;
    QString style = QQuickStyle::name();
    if (!style.isEmpty() && !QQuickStylePrivate::isUsingDefaultStyle())
        settings.setValue("style", style);
    else
        QQuickStyle::setStyle(settings.value("style").isValid() ? settings.value("style").toString() : "Imagine");

    if (QFontDatabase::addApplicationFont(":qt/qml/Testbench/fonts/fontello.ttf") == -1)
        qWarning() << "Failed to load fontawesome font";

    QQmlApplicationEngine engine;
    QObject::connect(
            &engine,
            &QQmlApplicationEngine::objectCreationFailed,
            &app,
            []() { QCoreApplication::exit(-1); },
            Qt::QueuedConnection);
    engine.loadFromModule("Testbench", "Main");

    return app.exec();
}

