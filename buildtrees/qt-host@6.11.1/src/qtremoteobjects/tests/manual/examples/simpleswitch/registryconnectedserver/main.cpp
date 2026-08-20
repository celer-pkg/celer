// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QCoreApplication>
#include "simpleswitch.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SimpleSwitch srcSwitch; // create simple switch

    QRemoteObjectRegistryHost regNode(QUrl(QStringLiteral("local:registry"))); // create node that hosts registy
    QRemoteObjectHost srcNode(QUrl(QStringLiteral("local:replica")), QUrl(QStringLiteral("local:registry"))); // create node that will host source and connect to registry

    //Note, you can add srcSwitch directly to regNode if desired.
    //We use two Nodes here, as the regNode could easily be in a third process.

    srcNode.enableRemoting(&srcSwitch); // enable remoting of source object

    return a.exec();
}

