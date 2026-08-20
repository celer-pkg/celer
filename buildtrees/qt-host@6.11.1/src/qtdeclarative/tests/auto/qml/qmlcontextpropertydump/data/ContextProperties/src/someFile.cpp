// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickView view;

    view.rootContext()->setContextProperty("myContextProperty1", QDateTime::currentDateTime());
    view.rootContext()->setContextProperty(
                u"myContextProperty2"_s,
                QDateTime::currentDateTime());
    registerContentProperties(view);

    view.loadFromModule("ContextProperty", "Main");
    view.show();

    return app.exec();
}
