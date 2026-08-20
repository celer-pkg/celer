// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include "echoclient.h"

int main(int argc, char *argv[])
{
    using namespace Qt::Literals::StringLiterals;
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("QtWebSockets example: echoclient");
    parser.addHelpOption();

    QCommandLineOption dbgOption(
            QStringList{ u"d"_s, u"debug"_s },
            QCoreApplication::translate("main", "Debug output [default: off]."));
    parser.addOption(dbgOption);
    QCommandLineOption hostnameOption(
            QStringList{ u"n"_s, u"hostname"_s },
            QCoreApplication::translate("main", "Hostname [default: localhost]."), "hostname",
            "localhost");
    parser.addOption(hostnameOption);
    QCommandLineOption portOption(QStringList{ u"p"_s, u"port"_s },
                                  QCoreApplication::translate("main", "Port [default: 1234]."),
                                  "port", "1234");
    parser.addOption(portOption);

    parser.process(a);
    bool debug = parser.isSet(dbgOption);
    bool ok = true;
    int port = parser.value(portOption).toInt(&ok);
    if (!ok || port < 1 || port > 65535) {
        qWarning("Port invalid, must be a number between 1 and 65535\n%s",
                 qPrintable(parser.helpText()));
        return 1;
    }
    QUrl url;
    url.setScheme(u"ws"_s);
    url.setHost(parser.value(hostnameOption));
    url.setPort(port);
    EchoClient client(url, debug);
    QObject::connect(&client, &EchoClient::closed, &a, &QCoreApplication::quit);

    return a.exec();
}
