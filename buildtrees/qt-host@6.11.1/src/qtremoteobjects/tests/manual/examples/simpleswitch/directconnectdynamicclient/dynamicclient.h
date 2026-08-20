// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef _DYNAMICCLIENT_H
#define _DYNAMICCLIENT_H

#include <QObject>
#include <QSharedPointer>

#include <QRemoteObjectNode>
#include <qremoteobjectdynamicreplica.h>

class DynamicClient : public QObject
{
    Q_OBJECT
public:
    DynamicClient(QSharedPointer<QRemoteObjectDynamicReplica> ptr);
    ~DynamicClient() override;

Q_SIGNALS:
    void echoSwitchState(bool switchState);// this signal is connected with server_slot(..) slot of source object and echoes back switch state received from source

public Q_SLOTS:
    void recSwitchState_slot(bool); // slot to receive source state
    void initConnection_slot();

private:
    bool clientSwitchState; // holds received server switch state
    QSharedPointer<QRemoteObjectDynamicReplica> reptr;// holds reference to replica
 };

#endif

