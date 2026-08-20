// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>
#include <QDateTime>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickView view;
    view.rootContext()->setContextProperty("myContextProperty", QDateTime::currentDateTime());
    view.loadFromModule("GenerateContextProperty", "Main");
    view.show();

    return app.exec();
}
