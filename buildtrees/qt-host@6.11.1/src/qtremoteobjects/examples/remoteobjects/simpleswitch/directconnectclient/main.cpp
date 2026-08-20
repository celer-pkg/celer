// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QCoreApplication>
#include "client.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    QSharedPointer<SimpleSwitchReplica> ptr;

    QRemoteObjectNode repNode; // create remote object node
    repNode.connectToNode(QUrl(QStringLiteral("local:replica"))); // connect with remote host node

    ptr.reset(repNode.acquire<SimpleSwitchReplica>()); // acquire replica of source from host node

    Client rswitch(ptr); // create client switch object and pass reference of replica to it

    return a.exec();
}
