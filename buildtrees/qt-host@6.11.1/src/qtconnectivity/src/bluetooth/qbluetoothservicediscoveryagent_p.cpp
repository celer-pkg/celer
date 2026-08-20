// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothservicediscoveryagent.h"
#include "qbluetoothservicediscoveryagent_p.h"
#ifndef QT_IOS_BLUETOOTH
#include "dummy/dummy_helper_p.h"
#endif

QT_BEGIN_NAMESPACE

QBluetoothServiceDiscoveryAgentPrivate::QBluetoothServiceDiscoveryAgentPrivate(
    QBluetoothServiceDiscoveryAgent *qp, const QBluetoothAddress &deviceAdapter)
    : error(QBluetoothServiceDiscoveryAgent::NoError),
      state(Inactive),
      mode(QBluetoothServiceDiscoveryAgent::MinimalDiscovery),
      singleDevice(false),
      q_ptr(qp)
{
#ifndef QT_IOS_BLUETOOTH
    printDummyWarning();
#endif
    Q_UNUSED(deviceAdapter);
}

QBluetoothServiceDiscoveryAgentPrivate::~QBluetoothServiceDiscoveryAgentPrivate()
{
}

void QBluetoothServiceDiscoveryAgentPrivate::start(const QBluetoothAddress &address)
{
    Q_UNUSED(address);
}

void QBluetoothServiceDiscoveryAgentPrivate::stop()
{
}

QT_END_NAMESPACE

#include "moc_qbluetoothservicediscoveryagent_p.cpp"
