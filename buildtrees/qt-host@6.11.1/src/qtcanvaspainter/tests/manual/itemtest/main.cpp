// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include "itemtest.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

#ifdef Q_OS_WIN
    qputenv("QT_QUICK_CONTROLS_STYLE", "FluentWinUI3");
#endif

    QQmlApplicationEngine engine;
    qmlRegisterType<HelloItem>("HelloItem", 1, 0, "HelloItem");
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
