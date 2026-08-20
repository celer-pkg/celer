// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "dialog.h"
#include "core.h"
#include "../shared/websocketclientwrapper.h"
#include "../shared/websockettransport.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QWebChannel>
#include <QWebSocketServer>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // setup the QWebSocketServer
    QWebSocketServer server(QStringLiteral("QWebChannel Standalone Example Server"), QWebSocketServer::NonSecureMode);
    if (!server.listen(QHostAddress::LocalHost, 12345)) {
        qFatal("Failed to open web socket server.");
        return 1;
    }

    // wrap WebSocket clients in QWebChannelAbstractTransport objects
    WebSocketClientWrapper clientWrapper(&server);

    // setup the channel
    QWebChannel channel;
    QObject::connect(&clientWrapper, &WebSocketClientWrapper::clientConnected,
                     &channel, &QWebChannel::connectTo);

    // setup the UI
    Dialog dialog;

    // setup the core and publish it to the QWebChannel
    Core core(&dialog);
    channel.registerObject(QStringLiteral("core"), &core);

    // open a browser window with the client HTML page
    QUrl url = QUrl::fromLocalFile(BUILD_DIR "/index.html");
    QDesktopServices::openUrl(url);

    dialog.displayMessage(Dialog::tr("Initialization complete, opening browser at %1.").arg(url.toDisplayString()));
    dialog.show();

    return app.exec();
}
