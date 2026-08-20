// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include "imagetestingitem.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qputenv("QT_RHI_LEAK_CHECK", "1");

    QQmlApplicationEngine engine;
    qmlRegisterType<ImageTestingItem>("ImageTestingItem", 1, 0, "ImageTestingItem");
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
