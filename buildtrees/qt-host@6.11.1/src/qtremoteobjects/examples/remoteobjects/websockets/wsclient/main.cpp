// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QTreeView>
#include <QApplication>
#include <QRemoteObjectNode>
#include <QAbstractItemModelReplica>
#include <QWebSocket>

#ifndef QT_NO_SSL
# include <QFile>
# include <QSslConfiguration>
# include <QSslKey>
#endif
#include "websocketiodevice.h"

int main(int argc, char **argv)
{

    QLoggingCategory::setFilterRules("qt.remoteobjects.debug=false\n"
                                     "qt.remoteobjects.warning=false\n"
                                     "qt.remoteobjects.models.debug=false\n"
                                     "qt.remoteobjects.models.debug=false");

    QApplication app(argc, argv);

    //! [0]
    QScopedPointer<QWebSocket> webSocket{new QWebSocket};
    WebSocketIoDevice socket(webSocket.data());
//! [0]

//! [1]
#ifndef QT_NO_SSL
    // Always use secure connections when available
    QSslConfiguration sslConf;
    QFile certFile(QStringLiteral(":/sslcert/client.crt"));
    if (!certFile.open(QIODevice::ReadOnly))
        qFatal("Can't open client.crt file");
    sslConf.setLocalCertificate(QSslCertificate{certFile.readAll()});

    QFile keyFile(QStringLiteral(":/sslcert/client.key"));
    if (!keyFile.open(QIODevice::ReadOnly))
        qFatal("Can't open client.key file");
    sslConf.setPrivateKey(QSslKey{keyFile.readAll(), QSsl::Rsa});

    sslConf.setPeerVerifyMode(QSslSocket::VerifyPeer);
    webSocket->setSslConfiguration(sslConf);
#endif
    //! [1]

    //! [2]
    QRemoteObjectNode node;
    node.addClientSideConnection(&socket);
    node.setHeartbeatInterval(1000);
    webSocket->open(QStringLiteral("ws://localhost:8088"));
    //! [2]

    //! [3]
    QTreeView view;
    view.setWindowTitle(QStringLiteral("RemoteView"));
    view.resize(640,480);
    QScopedPointer<QAbstractItemModelReplica> model(node.acquireModel(QStringLiteral("RemoteModel")));
    view.setModel(model.data());
    view.show();

    return app.exec();
    //! [3]
}
