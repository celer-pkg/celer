// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "timemodel.h"

#include <QCoreApplication>
#include <QSslConfiguration>
#include <QSslKey>
#include <QSslServer>

#include <QRemoteObjectHost>
/*
* http://stackoverflow.com/questions/7404163/windows-handling-ctrlc-in-different-thread
*/

void SigIntHandler()
{
    qDebug()<<"Ctrl-C received.  Quitting.";
    qApp->quit();
}

#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX) || defined(Q_OS_QNX)
#include <signal.h>

void unix_handler(int s)
{
    if (s==SIGINT)
        SigIntHandler();
}

#elif defined(Q_OS_WIN32)
#include <windows.h>

BOOL WINAPI WinHandler(DWORD CEvent)
{
    switch (CEvent)
    {
    case CTRL_C_EVENT:
        SigIntHandler();
        return TRUE;
    }
    return FALSE;
}
#endif

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    //! [0]
    auto config = QSslConfiguration::defaultConfiguration();
    config.setCaCertificates(QSslCertificate::fromPath(QStringLiteral(":/sslcert/rootCA.pem")));
    QFile certificateFile(QStringLiteral(":/sslcert/server.crt"));
    if (certificateFile.open(QIODevice::ReadOnly | QIODevice::Text))
        config.setLocalCertificate(QSslCertificate(certificateFile.readAll(), QSsl::Pem));
    else
        qFatal("Could not open certificate file");
    QFile keyFile(QStringLiteral(":/sslcert/server.key"));
    if (keyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QSslKey key(keyFile.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
        if (key.isNull())
            qFatal("Key is not valid");
        config.setPrivateKey(key);
    } else {
        qFatal("Could not open key file");
    }
    config.setPeerVerifyMode(QSslSocket::VerifyPeer);
    QSslConfiguration::setDefaultConfiguration(config);
    //! [0]

#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX) || defined(Q_OS_QNX)
    signal(SIGINT, &unix_handler);
#elif defined(Q_OS_WIN32)
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)WinHandler, TRUE);
#endif
    //! [1]
    QRemoteObjectHost host;
    QSslServer server;
    server.listen(QHostAddress::Any, 65511);
    host.setHostUrl(server.serverAddress().toString(), QRemoteObjectHost::AllowExternalRegistration);
    //! [1]

    //! [2]
    QObject::connect(&server, &QSslServer::errorOccurred,
                     [](QSslSocket *socket, QAbstractSocket::SocketError error) {
                         Q_UNUSED(socket);
                         qDebug() << "QSslServer::errorOccurred" << error;
                     });
    QObject::connect(&server, &QSslServer::pendingConnectionAvailable, [&server, &host]() {
        qDebug() << "New connection available";
        QSslSocket *socket = qobject_cast<QSslSocket *>(server.nextPendingConnection());
        Q_ASSERT(socket);
        QObject::connect(socket, &QSslSocket::errorOccurred,
                         [](QAbstractSocket::SocketError error) {
                             qDebug() << "QSslSocket::error" << error;
                         });
        host.addHostSideConnection(socket);
    });
    //! [2]

    //! [3]
    MinuteTimer timer;
    host.enableRemoting(&timer);
    //! [3]

    Q_UNUSED(timer)
    return app.exec();
}

