// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlFileSelector>
#include <QQuickView>
#include <QQmlApplicationEngine>


int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("QtProject");
    QCoreApplication::setApplicationName("Maroon");

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.loadFromModule(u"examples.Maroon", u"Main");

    return app.exec();
}
