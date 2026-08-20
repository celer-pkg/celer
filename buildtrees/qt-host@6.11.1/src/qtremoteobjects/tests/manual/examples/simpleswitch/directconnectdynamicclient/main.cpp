// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QCoreApplication>

#include "dynamicclient.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSharedPointer<QRemoteObjectDynamicReplica> ptr; // shared pointer to hold replica

    QRemoteObjectNode repNode; // create remote object node
    repNode.connectToNode(QUrl(QStringLiteral("local:replica"))); // connect with remote host node

    ptr.reset(repNode.acquireDynamic("SimpleSwitch")); // acquire replica of source from host node

    DynamicClient rswitch(ptr); // create client switch object and pass replica reference to it

    return a.exec();
}
