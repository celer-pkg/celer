// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QIcon::setThemeName("ndefeditor");

    QQmlApplicationEngine engine;

    QObject::connect(
            &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
            []() { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("NdefEditorModule", "Main");

    return app.exec();
}
