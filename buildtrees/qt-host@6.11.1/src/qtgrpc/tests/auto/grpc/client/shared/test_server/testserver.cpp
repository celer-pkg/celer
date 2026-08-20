// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testserverrunner.h"

#include <memory>

#include <QCommandLineParser>
#include <QDebug>

int main(int argc, char *argv[])
{
    QStringList argumentList;
    for (int i = 0; i < argc; ++i)
        argumentList.append(argv[i]);

    QCommandLineParser parser;
    parser.addOption({ "latency", "Expected latency of test server communication in ms.", "ms" });

    if (!parser.parse(argumentList)) {
        qCritical() << parser.errorText();
        return 1;
    }

    QString latencyString = parser.value("latency");
    if (latencyString.isEmpty()) {
        qCritical("Message latency is missing");
        return 2;
    }
    bool ok = false;
    auto latency = latencyString.toLongLong(&ok);
    if (!ok) {
        qCritical() << "Unable to parse message latency argument" << parser.value("latency");
        return 3;
    }

    auto server = std::make_unique<TestServer>();
    server->run(latency);
    return 0;
}
