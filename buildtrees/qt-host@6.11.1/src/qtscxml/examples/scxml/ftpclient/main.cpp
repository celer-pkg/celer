// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "ftpcontrolchannel.h"
#include "ftpdatachannel.h"
#include "simpleftp.h"

#include <QtCore/qcoreapplication.h>

#include <iostream>

struct Command {
    QString cmd;
    QString args;
};

int main(int argc, char *argv[])
{
    if (argc != 3) {
        qWarning() << "Usage: ftpclient <server> <file>";
        qWarning() << "For example: ftpclient ftp.gnu.org welcome.msg";
        return 1;
    }

    QString server = QString::fromLocal8Bit(argv[1]);
    QString file = QString::fromLocal8Bit(argv[2]);

    QCoreApplication app(argc, argv);
    FtpClient ftpClient;
    FtpDataChannel dataChannel;
    FtpControlChannel controlChannel;

    // Print all data retrieved from the server on the console.
    QObject::connect(&dataChannel, &FtpDataChannel::dataReceived,
                     [](const QByteArray &data) {
        std::cout << data.constData() << std::flush;
    });

    // Translate server replies into state machine events.
    QObject::connect(&controlChannel, &FtpControlChannel::reply, &ftpClient,
                     [&ftpClient](int code, const QString &parameters) {
        ftpClient.submitEvent(QString("reply.%1xx")
                              .arg(code / 100), parameters);
    });

    // Translate commands from the state machine into FTP control messages.
    ftpClient.connectToEvent("submit.cmd", &controlChannel,
                             [&controlChannel](const QScxmlEvent &event) {
        controlChannel.command(event.name().mid(11).toUtf8(),
                               event.data().toMap()["params"].toByteArray());
    });

    // Commands to be sent
    QList<Command> commands({
        {"cmd.USER", "anonymous"},// login
        {"cmd.PORT", ""},         // announce port for data connection,
                                  // args added below.
        {"cmd.RETR", file}        // retrieve a file
    });

    // When entering "B" state, send the next command.
    ftpClient.connectToState("B", QScxmlStateMachine::onEntry([&]() {
        if (commands.isEmpty()) {
            app.quit();
            return;
        }
        Command command = commands.takeFirst();
        qDebug() << "Posting command" << command.cmd << command.args;
        ftpClient.submitEvent(command.cmd, command.args);
    }));

    // If the server asks for a password, send one.
    ftpClient.connectToState("P", QScxmlStateMachine::onEntry([&ftpClient]() {
        qDebug() << "Sending password";
        ftpClient.submitEvent("cmd.PASS", QString());
    }));

    // Connect to our own local FTP server
    controlChannel.connectToServer(server);
    QObject::connect(&controlChannel, &FtpControlChannel::opened, &dataChannel,
                     [&](const QHostAddress &address, int) {
        dataChannel.listen(address);
        commands[1].args = dataChannel.portspec();
        ftpClient.start();
    });

    return app.exec();
}
