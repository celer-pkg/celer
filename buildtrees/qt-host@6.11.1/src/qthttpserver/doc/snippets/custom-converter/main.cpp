// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QHttpServer>
#include <QHttpServerResponse>

#include <QCoreApplication>
#include <QString>
#include <QTcpServer>
#include <QTimer>
#include <QMetaType>

//! [Define class]
struct CustomArg {
    int data = 10;

    CustomArg() {} ;
    CustomArg(const QString &urlArg) : data(urlArg.toInt()) {}
};
//! [Define class]

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QHttpServer server;
    auto tcpServer = std::make_unique<QTcpServer>();
    if (!tcpServer->listen(QHostAddress::Any, 1999) || !server.bind(tcpServer.get())) {
        qWarning() << "Server failed to listen on port 1999.";
        return -1;
    }
    tcpServer.release();

    //! [Add converter and route]
    server.router()->addConverter<CustomArg>(u"[+-]?\\d+");
    server.route("/customTest/<arg>", [] (const CustomArg custom) { return QString::number(custom.data); });
    //! [Add converter and route]

    qInfo().noquote() << "Server listening on port 1999.";

    return app.exec();
}
