// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef MYTESTSERVER_H
#define MYTESTSERVER_H

#include <QTimer>

#include <QtRemoteObjects/qremoteobjectnode.h>
#include <QtRemoteObjects/qremoteobjectsource.h>

#include "rep_MyInterface_source.h"

class MyTestServer : public MyInterfaceSimpleSource
{
    Q_OBJECT

public:
    MyTestServer(QObject *parent = nullptr);
    ~MyTestServer() override;

public Q_SLOTS:
    bool start() override;
    bool stop() override;
    bool quit() override;
    void testEnumParamsInSlots(Enum1 enumSlotParam, bool slotParam2, int __repc_variable_1) override;
    void testExtPODListSlot(const QList<ExtPOD> &l) override;

Q_SIGNALS:
    void quitApp();
};

#endif // MYTESTSERVER_H
