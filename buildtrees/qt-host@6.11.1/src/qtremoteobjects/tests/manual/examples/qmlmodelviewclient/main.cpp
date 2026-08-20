// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QRemoteObjectNode>
#include <QAbstractItemModelReplica>

int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules("qt.remoteobjects.debug=false\n"
                                     "qt.remoteobjects.warning=false\n"
                                     "qt.remoteobjects.models.debug=false\n"
                                     "qt.remoteobjects.models.debug=false");

    QGuiApplication app(argc, argv);

    QRemoteObjectNode node(QUrl(QStringLiteral("local:registry")));
    QQmlApplicationEngine engine;
    QScopedPointer<QAbstractItemModelReplica> model(node.acquireModel(QStringLiteral("RemoteModel")));
    engine.rootContext()->setContextProperty("remoteModel", model.data());
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
