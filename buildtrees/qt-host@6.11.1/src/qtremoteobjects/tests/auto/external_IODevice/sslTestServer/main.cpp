// Copyright (C) 2018 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "pingpong.h"
#include "sslserver.h"

#include <QCoreApplication>
#include <QDebug>
#include <QSslConfiguration>

#include <QRemoteObjectHost>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QRemoteObjectHost host;
    auto config = QSslConfiguration::defaultConfiguration();
    config.setCaCertificates(QSslCertificate::fromPath(QStringLiteral(":/sslcert/rootCA.pem")));
    QSslConfiguration::setDefaultConfiguration(config);
    SslServer server;
    server.listen(QHostAddress::Any, 65111);
    host.setHostUrl(QUrl{server.serverAddress().toString()},
                    QRemoteObjectHost::AllowExternalRegistration);
    QObject::connect(&server, &SslServer::encryptedSocketReady, &server, [&host](QSslSocket *socket){
        QObject::connect(socket, &QSslSocket::errorOccurred,
                socket, [](QAbstractSocket::SocketError error){
            qDebug() << "QSslSocket::error" << error;
            exit(1);
        });
        host.addHostSideConnection(socket);
    });

    PingPong pp;
    host.enableRemoting(&pp);
    return app.exec();
}
