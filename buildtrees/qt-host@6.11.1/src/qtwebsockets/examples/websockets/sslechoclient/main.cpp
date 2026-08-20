// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include "sslechoclient.h"

int main(int argc, char *argv[])
{
    using namespace Qt::Literals::StringLiterals;
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("QtWebSockets example: sslechoclient");
    parser.addHelpOption();

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
    bool ok = true;
    int port = parser.value(portOption).toInt(&ok);
    if (!ok || port < 1 || port > 65535) {
        qWarning("Port invalid, must be a number between 1 and 65535\n%s",
                 qPrintable(parser.helpText()));
        return 1;
    }
    QUrl url;
    url.setScheme(u"wss"_s);
    url.setHost(parser.value(hostnameOption));
    url.setPort(port);
    SslEchoClient client(url);
    Q_UNUSED(client);

    return a.exec();
}
