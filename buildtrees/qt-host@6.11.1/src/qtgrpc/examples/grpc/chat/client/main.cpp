// Copyright (C) 2023 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "chatengine.h"

#include <QtProtobufQtCoreTypes>

#include <QtQml/QQmlApplicationEngine>

#include <QtGui/QFontDatabase>
#include <QtGui/QGuiApplication>
#include <QtGui/QIcon>

int main(int argc, char *argv[])
{
    QtProtobuf::registerProtobufQtCoreTypes();

    QGuiApplication app(argc, argv);
    QGuiApplication::setWindowIcon(QIcon(":/res/qtchat_logo.png"));
    QGuiApplication::setApplicationName("QtGrpcChat");
    QGuiApplication::setOrganizationName("The Qt Company");
    QGuiApplication::setOrganizationDomain("qt.io");
#ifdef USE_EMOJI_FONT
    QFontDatabase::addApplicationFont(":/NotoColorEmoji.ttf");
#endif

    QQmlApplicationEngine engine;
    engine.loadFromModule("QtGrpcChat", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;

    // Install the engine as event filter. We use it to detect inactivity
    auto *chatEngine = engine.singletonInstance<ChatEngine*>("QtGrpcChat", "ChatEngine");
    app.installEventFilter(chatEngine);

    return QGuiApplication::exec();
}
