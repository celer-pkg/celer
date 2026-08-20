// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QTreeView>
#include <QApplication>
#include <QRemoteObjectNode>
#include <QAbstractItemModelReplica>

int main(int argc, char **argv)
{

    QLoggingCategory::setFilterRules("qt.remoteobjects.debug=false\n"
                                     "qt.remoteobjects.warning=false\n"
                                     "qt.remoteobjects.models.debug=false\n"
                                     "qt.remoteobjects.models.debug=false");

    QApplication app(argc, argv);

//! [ObjectNode creation]
    QRemoteObjectNode node(QUrl(QStringLiteral("local:registry")));
    node.setHeartbeatInterval(1000);
//! [ObjectNode creation]
//! [Model acquisition]
    QScopedPointer<QAbstractItemModelReplica> model(node.acquireModel(QStringLiteral("RemoteModel")));
//! [Model acquisition]

//! [QTreeView-creation]
    QTreeView view;
    view.setWindowTitle(QStringLiteral("RemoteView"));
    view.resize(640,480);
    view.setModel(model.data());
    view.show();
//! [QTreeView-creation]

    return app.exec();
}
