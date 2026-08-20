// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "dynamicclient.h"
 #include <QMetaMethod>

// constructor
DynamicClient::DynamicClient(QSharedPointer<QRemoteObjectDynamicReplica> ptr) :
    QObject(nullptr), clientSwitchState(false), reptr(ptr)
{
    //connect signal for replica valid changed with signal slot initialization
    QObject::connect(reptr.data(), &QRemoteObjectDynamicReplica::initialized, this,
                     &DynamicClient::initConnection_slot);
}

//destructor
DynamicClient::~DynamicClient()
{

}

// Function to initialize connections between slots and signals
void DynamicClient::initConnection_slot()
{

    // connect source replica signal currStateChanged() with client's recSwitchState() slot to receive source's current state
   QObject::connect(reptr.data(), SIGNAL(currStateChanged(bool)), this, SLOT(recSwitchState_slot(bool)));
   // connect client's echoSwitchState(..) signal with replica's server_slot(..) to echo back received state
   QObject::connect(this, SIGNAL(echoSwitchState(bool)),reptr.data(), SLOT(server_slot(bool)));
}


void DynamicClient::recSwitchState_slot(bool value)
{
   clientSwitchState = reptr->property("currState").toBool(); // use replica property to get "currState" from source
   qDebug() << "Received source state " << value << clientSwitchState;
   Q_EMIT echoSwitchState(clientSwitchState); // Emit signal to echo received state back to server
}

