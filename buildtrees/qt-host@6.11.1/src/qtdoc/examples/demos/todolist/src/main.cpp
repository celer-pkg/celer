// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QQuickStyle>

#include "app_environment.h"
#include "import_qml_plugins.h"

int main(int argc, char *argv[])
{
    set_qt_environment();
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("ToDoList");
    QGuiApplication::setOrganizationName("QtProject");

    QSettings settings;
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE") &&
        settings.value("style").toString().isEmpty()) {
#if defined(Q_OS_MACOS)
        QQuickStyle::setStyle(QString("iOS"));
#elif defined(Q_OS_IOS)
        QQuickStyle::setStyle(QString("iOS"));
#elif defined(Q_OS_WINDOWS)
        QQuickStyle::setStyle(QString("Windows"));
#elif defined(Q_OS_ANDROID)
        QQuickStyle::setStyle(QString("Material"));
#endif
    } else {
        QQuickStyle::setStyle(settings.value("style").toString());
    }

    const QString styleInSettings = settings.value("style").toString();
    if (styleInSettings.isEmpty())
        settings.setValue(QString("style"), QQuickStyle::name());

    QStringList builtInStyles = { QString("CustomStyle"), QString("Basic"),
                                  QString("Material"), QString("Universal") };
#if defined(Q_OS_MACOS)
    builtInStyles << QString("iOS");
#elif defined(Q_OS_IOS)
    builtInStyles << QString("iOS");
#elif defined(Q_OS_WINDOWS)
    builtInStyles << QString("Windows");
#endif

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QGuiApplication::quit);

    engine.setInitialProperties({{ "builtInStyles", builtInStyles }});
    engine.loadFromModule("Main", "Main");

    return app.exec();
}
