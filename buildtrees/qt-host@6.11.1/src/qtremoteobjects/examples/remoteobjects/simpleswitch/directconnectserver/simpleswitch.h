// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SIMPLESWITCH_H
#define SIMPLESWITCH_H

#include "rep_simpleswitch_source.h"

class SimpleSwitch : public SimpleSwitchSimpleSource
{
    Q_OBJECT
public:
    SimpleSwitch(QObject *parent = nullptr);
    ~SimpleSwitch() override;
    void server_slot(bool clientState) override;
public Q_SLOTS:
    void timeout_slot();
private:
    QTimer *stateChangeTimer;
};

#endif
