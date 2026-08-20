// Copyright (C) 2018 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>

QT_BEGIN_NAMESPACE
class QSslSocket;
QT_END_NAMESPACE

class SslServer : public QTcpServer
{
    Q_OBJECT
public:
    SslServer(QObject *parent=nullptr);
    void incomingConnection(qintptr socketDescriptor) override;

signals:
    void encryptedSocketReady(QSslSocket *socket);
};


#endif // SSLSERVER_H
