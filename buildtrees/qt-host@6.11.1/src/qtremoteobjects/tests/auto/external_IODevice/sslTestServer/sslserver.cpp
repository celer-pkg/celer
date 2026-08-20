// Copyright (C) 2018 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "sslserver.h"
#include <QSslSocket>

SslServer::SslServer(QObject *parent)
    : QTcpServer(parent)
{}


void SslServer::incomingConnection(qintptr socketDescriptor)
{
    auto serverSocket = new QSslSocket;
    if (serverSocket->setSocketDescriptor(socketDescriptor)) {
        addPendingConnection(serverSocket);
        connect(serverSocket, &QSslSocket::encrypted, this, [this, serverSocket] {
           Q_EMIT encryptedSocketReady(serverSocket);
        });
        connect(serverSocket, static_cast<void (QSslSocket::*)(const QList<QSslError>&)>(&QSslSocket::sslErrors),
                this, [serverSocket](const QList<QSslError>& errors){
            qWarning() << "Error:" << serverSocket << errors;
            delete serverSocket;
        });
        serverSocket->setPeerVerifyMode(QSslSocket::VerifyPeer);
        serverSocket->setLocalCertificate(QStringLiteral(":/sslcert/server.crt"));
        serverSocket->setPrivateKey(QStringLiteral(":/sslcert/server.key"));
        serverSocket->startServerEncryption();
    } else {
        delete serverSocket;
    }
}
