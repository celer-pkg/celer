// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qdebug.h>

#include "mytestserver.h"
#include "rep_MyInterface_source.h"

MyTestServer::MyTestServer(QObject *parent)
    : MyInterfaceSimpleSource(parent)
{
    qDebug() << "Server started";
    setInitialValue(18);
}

MyTestServer::~MyTestServer()
{
    qDebug() << "Server stopped";
}

bool MyTestServer::start()
{
    setStarted(true);
    return true;
}

bool MyTestServer::stop()
{
    setStarted(false);
    return true;
}

bool MyTestServer::quit()
{
    emit quitApp();
    return true;
}

void MyTestServer::testEnumParamsInSlots(Enum1 enumSlotParam, bool slotParam2, int number)
{
    setEnum1(enumSlotParam);
    setStarted(slotParam2);
    emit testEnumParamsInSignals(enum1(), started(), QString::number(number));
}

void MyTestServer::testExtPODListSlot(const QList<ExtPOD> &l)
{
    emit testExtPODListSignal(l);
}
